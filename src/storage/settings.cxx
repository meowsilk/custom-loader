#include "storage/settings.hxx"

#include "storage/atomic_file.hxx"

#include <cstddef>
#include <exception>
#include <vector>

namespace loader::storage
{
namespace
{

constexpr std::string_view kLanguageKey = "language=";
constexpr std::size_t kMaxCodeLength = 16;

}

std::optional<std::string> Settings::language() const
{
    const std::optional<std::vector<std::byte>> bytes = readFile(paths_.settingsFile());
    if (!bytes)
        return std::nullopt;

    std::string content;
    content.reserve(bytes->size());
    for (const std::byte value : *bytes)
        content.push_back(static_cast<char>(value));

    const std::size_t start = content.find(kLanguageKey);
    if (start == std::string::npos)
        return std::nullopt;

    const std::size_t valueStart = start + kLanguageKey.size();
    const std::size_t end = content.find_first_of("\r\n", valueStart);
    std::string code =
        content.substr(valueStart, end == std::string::npos ? std::string::npos : end - valueStart);
    if (code.empty() || code.size() > kMaxCodeLength)
        return std::nullopt;
    return code;
}

bool Settings::setLanguage(std::string_view code) const
{
    std::string content(kLanguageKey);
    content.append(code);
    content.push_back('\n');

    std::vector<std::byte> bytes;
    bytes.reserve(content.size());
    for (const char value : content)
        bytes.push_back(static_cast<std::byte>(value));

    try
    {
        writeFileAtomically(paths_.settingsFile(), bytes);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}

}
