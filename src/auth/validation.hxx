#pragma once

#include <string_view>

namespace loader::auth
{

inline constexpr std::size_t kMinUsernameLength = 3;
inline constexpr std::size_t kMaxUsernameLength = 32;
inline constexpr std::size_t kMinPasswordLength = 6;
inline constexpr std::size_t kMaxPasswordLength = 128;
inline constexpr std::size_t kLicenseKeyLength = 16;

enum class FieldIssue
{
    None,
    Empty,
    TooShort,
    TooLong,
    InvalidCharacters,
    Mismatch,
    Incomplete,
};

FieldIssue validateUsername(std::u32string_view username);
FieldIssue validatePassword(std::u32string_view password);
FieldIssue validatePasswordConfirmation(std::u32string_view password, std::u32string_view confirmation);
FieldIssue validateLicenseKey(std::u32string_view normalizedKey);

}
