#include "core/utf.hxx"

#include <cstdint>

namespace loader::core::utf
{
namespace
{

constexpr char32_t kMaxCodePoint = 0x10FFFF;
constexpr char32_t kSurrogateFirst = 0xD800;
constexpr char32_t kSurrogateLast = 0xDFFF;

constexpr bool isScalarValue(char32_t cp)
{
    return cp <= kMaxCodePoint && (cp < kSurrogateFirst || cp > kSurrogateLast);
}

struct Utf8Sequence
{
    int length;
    char32_t minimum;
    char32_t payloadMask;
};

constexpr Utf8Sequence sequenceFor(unsigned char lead)
{
    if (lead < 0x80)
        return {1, 0, 0x7F};
    if ((lead & 0xE0) == 0xC0)
        return {2, 0x80, 0x1F};
    if ((lead & 0xF0) == 0xE0)
        return {3, 0x800, 0x0F};
    if ((lead & 0xF8) == 0xF0)
        return {4, 0x10000, 0x07};
    return {0, 0, 0};
}

}

std::u32string decodeUtf8(std::string_view text)
{
    std::u32string result;
    result.reserve(text.size());

    std::size_t i = 0;
    while (i < text.size())
    {
        const auto lead = static_cast<unsigned char>(text[i]);
        const Utf8Sequence sequence = sequenceFor(lead);
        if (sequence.length == 0 || i + static_cast<std::size_t>(sequence.length) > text.size())
        {
            result.push_back(kReplacement);
            ++i;
            continue;
        }

        char32_t codePoint = lead & sequence.payloadMask;
        bool wellFormed = true;
        for (int k = 1; k < sequence.length; ++k)
        {
            const auto next = static_cast<unsigned char>(text[i + static_cast<std::size_t>(k)]);
            if ((next & 0xC0) != 0x80)
            {
                wellFormed = false;
                break;
            }
            codePoint = (codePoint << 6) | (next & 0x3F);
        }

        if (!wellFormed || codePoint < sequence.minimum || !isScalarValue(codePoint))
        {
            result.push_back(kReplacement);
            ++i;
            continue;
        }

        result.push_back(codePoint);
        i += static_cast<std::size_t>(sequence.length);
    }
    return result;
}

std::string encodeUtf8(std::u32string_view text)
{
    std::string result;
    result.reserve(text.size());

    for (char32_t cp : text)
    {
        if (!isScalarValue(cp))
            cp = kReplacement;

        if (cp < 0x80)
        {
            result.push_back(static_cast<char>(cp));
        }
        else if (cp < 0x800)
        {
            result.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else if (cp < 0x10000)
        {
            result.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
        else
        {
            result.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return result;
}

std::u32string decodeUtf16(std::wstring_view text)
{
    std::u32string result;
    result.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        const auto unit = static_cast<char32_t>(static_cast<std::uint16_t>(text[i]));
        const bool highSurrogate = unit >= 0xD800 && unit <= 0xDBFF;
        const bool lowSurrogate = unit >= 0xDC00 && unit <= 0xDFFF;

        if (lowSurrogate)
        {
            result.push_back(kReplacement);
        }
        else if (highSurrogate)
        {
            const bool hasPair = i + 1 < text.size() && static_cast<std::uint16_t>(text[i + 1]) >= 0xDC00 &&
                                 static_cast<std::uint16_t>(text[i + 1]) <= 0xDFFF;
            if (!hasPair)
            {
                result.push_back(kReplacement);
                continue;
            }
            const auto low = static_cast<char32_t>(static_cast<std::uint16_t>(text[++i]));
            result.push_back(0x10000 + ((unit - 0xD800) << 10) + (low - 0xDC00));
        }
        else
        {
            result.push_back(unit);
        }
    }
    return result;
}

std::wstring encodeUtf16(std::u32string_view text)
{
    std::wstring result;
    result.reserve(text.size());

    for (char32_t cp : text)
    {
        if (!isScalarValue(cp))
            cp = kReplacement;

        if (cp < 0x10000)
        {
            result.push_back(static_cast<wchar_t>(cp));
        }
        else
        {
            const char32_t offset = cp - 0x10000;
            result.push_back(static_cast<wchar_t>(0xD800 + (offset >> 10)));
            result.push_back(static_cast<wchar_t>(0xDC00 + (offset & 0x3FF)));
        }
    }
    return result;
}

std::wstring widen(std::string_view utf8)
{
    return encodeUtf16(decodeUtf8(utf8));
}

std::string narrow(std::wstring_view utf16)
{
    return encodeUtf8(decodeUtf16(utf16));
}

}
