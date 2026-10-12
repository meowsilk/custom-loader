#pragma once

#include <string>
#include <string_view>

namespace loader::core::utf
{

inline constexpr char32_t kReplacement = U'�';

std::u32string decodeUtf8(std::string_view text);
std::string encodeUtf8(std::u32string_view text);
std::u32string decodeUtf16(std::wstring_view text);
std::wstring encodeUtf16(std::u32string_view text);

std::wstring widen(std::string_view utf8);
std::string narrow(std::wstring_view utf16);

}
