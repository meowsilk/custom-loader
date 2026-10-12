#include "auth/validation.hxx"

namespace loader::auth
{
namespace
{

bool isUsernameCharacter(char32_t character)
{
    return (character >= U'a' && character <= U'z') || (character >= U'A' && character <= U'Z') ||
           (character >= U'0' && character <= U'9') || character == U'.' || character == U'_' ||
           character == U'-';
}

bool isLicenseCharacter(char32_t character)
{
    return (character >= U'A' && character <= U'Z') || (character >= U'0' && character <= U'9');
}

}

FieldIssue validateUsername(std::u32string_view username)
{
    if (username.empty())
        return FieldIssue::Empty;
    if (username.size() < kMinUsernameLength)
        return FieldIssue::TooShort;
    if (username.size() > kMaxUsernameLength)
        return FieldIssue::TooLong;
    for (const char32_t character : username)
    {
        if (!isUsernameCharacter(character))
            return FieldIssue::InvalidCharacters;
    }
    return FieldIssue::None;
}

FieldIssue validatePassword(std::u32string_view password)
{
    if (password.empty())
        return FieldIssue::Empty;
    if (password.size() < kMinPasswordLength)
        return FieldIssue::TooShort;
    if (password.size() > kMaxPasswordLength)
        return FieldIssue::TooLong;
    return FieldIssue::None;
}

FieldIssue validatePasswordConfirmation(std::u32string_view password, std::u32string_view confirmation)
{
    if (confirmation.empty())
        return FieldIssue::Empty;
    return password == confirmation ? FieldIssue::None : FieldIssue::Mismatch;
}

FieldIssue validateLicenseKey(std::u32string_view normalizedKey)
{
    if (normalizedKey.empty())
        return FieldIssue::Empty;
    for (const char32_t character : normalizedKey)
    {
        if (!isLicenseCharacter(character))
            return FieldIssue::InvalidCharacters;
    }
    return normalizedKey.size() == kLicenseKeyLength ? FieldIssue::None : FieldIssue::Incomplete;
}

}
