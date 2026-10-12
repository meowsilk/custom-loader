#include "storage/dpapi.hxx"

#include <dpapi.h>

#include <Windows.h>

#include <cstring>

namespace loader::storage
{
namespace
{

constexpr char kEntropy[] = "loader.gg/vault/v1";

DATA_BLOB entropyBlob()
{
    return {static_cast<DWORD>(sizeof(kEntropy) - 1), reinterpret_cast<BYTE*>(const_cast<char*>(kEntropy))};
}

}

std::optional<std::vector<std::byte>> protectForCurrentUser(std::span<const std::byte> plaintext)
{
    DATA_BLOB input{static_cast<DWORD>(plaintext.size()),
                    reinterpret_cast<BYTE*>(const_cast<std::byte*>(plaintext.data()))};
    DATA_BLOB entropy = entropyBlob();
    DATA_BLOB output{};

    if (!CryptProtectData(
            &input, L"loader.gg credentials", &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output))
        return std::nullopt;

    std::vector<std::byte> result(output.cbData);
    std::memcpy(result.data(), output.pbData, output.cbData);
    LocalFree(output.pbData);
    return result;
}

std::optional<SecureBytes> unprotectForCurrentUser(std::span<const std::byte> ciphertext)
{
    DATA_BLOB input{static_cast<DWORD>(ciphertext.size()),
                    reinterpret_cast<BYTE*>(const_cast<std::byte*>(ciphertext.data()))};
    DATA_BLOB entropy = entropyBlob();
    DATA_BLOB output{};

    if (!CryptUnprotectData(&input, nullptr, &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output))
        return std::nullopt;

    SecureBytes result(output.cbData);
    std::memcpy(result.data(), output.pbData, output.cbData);
    SecureZeroMemory(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return result;
}

}
