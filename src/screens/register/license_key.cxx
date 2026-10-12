#include "screens/register/license_key.hxx"

#include "auth/validation.hxx"

namespace loader::screens
{
namespace
{

constexpr std::size_t kGroupSize = 4;

bool isAlphanumeric(char32_t character)
{
    return (character >= U'A' && character <= U'Z') || (character >= U'a' && character <= U'z') ||
           (character >= U'0' && character <= U'9');
}

char32_t uppercase(char32_t character)
{
    return character >= U'a' && character <= U'z' ? character - (U'a' - U'A') : character;
}

}

std::u32string normalizeLicenseKey(std::u32string_view raw)
{
    std::u32string result;
    for (const char32_t character : raw)
    {
        if (result.size() == auth::kLicenseKeyLength)
            break;
        if (isAlphanumeric(character))
            result.push_back(uppercase(character));
    }
    return result;
}

std::u32string formatLicenseKey(std::u32string_view normalized)
{
    std::u32string result;
    result.reserve(normalized.size() + normalized.size() / kGroupSize);
    for (std::size_t i = 0; i < normalized.size(); ++i)
    {
        if (i > 0 && i % kGroupSize == 0)
            result.push_back(U'-');
        result.push_back(normalized[i]);
    }
    return result;
}

std::u32string LicenseKeyTransform::apply(std::u32string_view text, std::size_t& caret) const
{
    std::size_t significantBeforeCaret = 0;
    for (std::size_t i = 0; i < caret && i < text.size(); ++i)
        significantBeforeCaret += isAlphanumeric(text[i]) ? 1 : 0;

    const std::u32string normalized = normalizeLicenseKey(text);
    significantBeforeCaret = std::min(significantBeforeCaret, normalized.size());

    const std::u32string formatted = formatLicenseKey(normalized);
    caret =
        significantBeforeCaret == 0 ? 0 : significantBeforeCaret + (significantBeforeCaret - 1) / kGroupSize;
    return formatted;
}

}
