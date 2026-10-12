#include "storage/credential_vault.hxx"

#include "core/utf.hxx"
#include "storage/atomic_file.hxx"
#include "storage/dpapi.hxx"

#include <array>
#include <cstdint>
#include <cstring>

namespace loader::storage
{
namespace
{

constexpr std::array<std::byte, 4> kMagic = {std::byte{'L'}, std::byte{'G'}, std::byte{'V'}, std::byte{'1'}};
constexpr std::uint8_t kPayloadVersion = 1;
constexpr std::uint8_t kSecretKindPassword = 1;
constexpr std::size_t kMaxFieldBytes = 1024;

void appendBytes(SecureBytes& buffer, const void* data, std::size_t size)
{
    const auto* bytes = static_cast<const std::byte*>(data);
    buffer.insert(buffer.end(), bytes, bytes + size);
}

void appendUtf8(SecureBytes& buffer, char32_t codePoint)
{
    const auto push = [&](unsigned value) { buffer.push_back(static_cast<std::byte>(value)); };
    if (codePoint < 0x80)
    {
        push(codePoint);
    }
    else if (codePoint < 0x800)
    {
        push(0xC0 | (codePoint >> 6));
        push(0x80 | (codePoint & 0x3F));
    }
    else if (codePoint < 0x10000)
    {
        push(0xE0 | (codePoint >> 12));
        push(0x80 | ((codePoint >> 6) & 0x3F));
        push(0x80 | (codePoint & 0x3F));
    }
    else
    {
        push(0xF0 | (codePoint >> 18));
        push(0x80 | ((codePoint >> 12) & 0x3F));
        push(0x80 | ((codePoint >> 6) & 0x3F));
        push(0x80 | (codePoint & 0x3F));
    }
}

void appendField(SecureBytes& buffer, std::u32string_view text)
{
    const std::size_t lengthOffset = buffer.size();
    const std::uint16_t placeholder = 0;
    appendBytes(buffer, &placeholder, sizeof(placeholder));

    const std::size_t start = buffer.size();
    for (const char32_t codePoint : text)
        appendUtf8(buffer, codePoint);

    const auto length = static_cast<std::uint16_t>(buffer.size() - start);
    std::memcpy(buffer.data() + lengthOffset, &length, sizeof(length));
}

class Reader
{
public:
    explicit Reader(std::span<const std::byte> bytes) : bytes_(bytes) {}

    bool read(void* destination, std::size_t size)
    {
        if (offset_ + size > bytes_.size())
            return false;
        std::memcpy(destination, bytes_.data() + offset_, size);
        offset_ += size;
        return true;
    }

    std::optional<std::span<const std::byte>> field()
    {
        std::uint16_t length = 0;
        if (!read(&length, sizeof(length)) || length > kMaxFieldBytes || offset_ + length > bytes_.size())
            return std::nullopt;
        const std::span<const std::byte> result = bytes_.subspan(offset_, length);
        offset_ += length;
        return result;
    }

private:
    std::span<const std::byte> bytes_;
    std::size_t offset_ = 0;
};

bool decodeSecret(std::span<const std::byte> bytes, core::SecretString& output)
{
    core::SecureString32 decoded;
    decoded.reserve(bytes.size());

    for (std::size_t i = 0; i < bytes.size();)
    {
        const auto lead = static_cast<unsigned>(bytes[i]);
        std::size_t length = 1;
        char32_t codePoint = lead;
        if (lead >= 0xF0)
        {
            length = 4;
            codePoint = lead & 0x07;
        }
        else if (lead >= 0xE0)
        {
            length = 3;
            codePoint = lead & 0x0F;
        }
        else if (lead >= 0xC0)
        {
            length = 2;
            codePoint = lead & 0x1F;
        }
        else if (lead >= 0x80)
        {
            return false;
        }

        if (i + length > bytes.size())
            return false;
        for (std::size_t k = 1; k < length; ++k)
        {
            const auto next = static_cast<unsigned>(bytes[i + k]);
            if ((next & 0xC0) != 0x80)
                return false;
            codePoint = (codePoint << 6) | (next & 0x3F);
        }
        decoded.push_back(codePoint);
        i += length;
    }

    output.assign(decoded);
    SecureZeroMemory(decoded.data(), decoded.size() * sizeof(char32_t));
    return true;
}

}

bool CredentialVault::exists() const
{
    return std::filesystem::exists(paths_.vaultFile());
}

bool CredentialVault::save(std::u32string_view username, std::u32string_view secret) const
{
    SecureBytes payload;
    payload.reserve(512);
    payload.push_back(static_cast<std::byte>(kPayloadVersion));
    payload.push_back(static_cast<std::byte>(kSecretKindPassword));
    appendField(payload, username);
    appendField(payload, secret);

    const std::optional<std::vector<std::byte>> blob = protectForCurrentUser(payload);
    if (!blob)
        return false;

    std::vector<std::byte> file(kMagic.begin(), kMagic.end());
    const auto blobSize = static_cast<std::uint32_t>(blob->size());
    file.resize(file.size() + sizeof(blobSize));
    std::memcpy(file.data() + kMagic.size(), &blobSize, sizeof(blobSize));
    file.insert(file.end(), blob->begin(), blob->end());

    try
    {
        writeFileAtomically(paths_.vaultFile(), file);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}

std::optional<StoredCredentials> CredentialVault::load() const
{
    const std::optional<std::vector<std::byte>> file = readFile(paths_.vaultFile());
    if (!file)
        return std::nullopt;

    Reader header(*file);
    std::array<std::byte, 4> magic{};
    std::uint32_t blobSize = 0;
    if (!header.read(magic.data(), magic.size()) || magic != kMagic ||
        !header.read(&blobSize, sizeof(blobSize)))
        return std::nullopt;
    if (file->size() != kMagic.size() + sizeof(blobSize) + blobSize)
        return std::nullopt;

    const std::span<const std::byte> blob = std::span(*file).subspan(kMagic.size() + sizeof(blobSize));
    const std::optional<SecureBytes> payload = unprotectForCurrentUser(blob);
    if (!payload)
        return std::nullopt;

    Reader reader(*payload);
    std::uint8_t version = 0;
    std::uint8_t kind = 0;
    if (!reader.read(&version, 1) || version != kPayloadVersion || !reader.read(&kind, 1) ||
        kind != kSecretKindPassword)
        return std::nullopt;

    const auto username = reader.field();
    const auto secret = reader.field();
    if (!username || !secret)
        return std::nullopt;

    StoredCredentials result;
    result.username = core::utf::decodeUtf8(
        std::string_view(reinterpret_cast<const char*>(username->data()), username->size()));
    if (!decodeSecret(*secret, result.secret))
        return std::nullopt;
    return result;
}

void CredentialVault::remove() const
{
    removeFileIfPresent(paths_.vaultFile());
}

}
