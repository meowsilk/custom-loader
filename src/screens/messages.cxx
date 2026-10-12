#include "screens/messages.hxx"

namespace loader::screens
{

using auth::AuthError;
using auth::FieldIssue;
using ui::Str;

Str describeUsernameIssue(FieldIssue issue)
{
    switch (issue)
    {
    case FieldIssue::Empty:
        return Str::UsernameEmpty;
    case FieldIssue::TooShort:
        return Str::UsernameTooShort;
    case FieldIssue::TooLong:
        return Str::UsernameTooLong;
    case FieldIssue::InvalidCharacters:
        return Str::UsernameInvalid;
    default:
        return Str::UsernameCheck;
    }
}

Str describePasswordIssue(FieldIssue issue)
{
    switch (issue)
    {
    case FieldIssue::Empty:
        return Str::PasswordEmpty;
    case FieldIssue::TooShort:
        return Str::PasswordTooShort;
    case FieldIssue::TooLong:
        return Str::PasswordTooLong;
    default:
        return Str::PasswordCheck;
    }
}

Str describeConfirmationIssue(FieldIssue issue)
{
    switch (issue)
    {
    case FieldIssue::Empty:
        return Str::ConfirmEmpty;
    case FieldIssue::Mismatch:
        return Str::ConfirmMismatch;
    default:
        return Str::ConfirmCheck;
    }
}

Str describeLicenseIssue(FieldIssue issue)
{
    switch (issue)
    {
    case FieldIssue::Empty:
        return Str::LicenseEmpty;
    case FieldIssue::Incomplete:
    case FieldIssue::TooShort:
    case FieldIssue::TooLong:
        return Str::LicenseLength;
    case FieldIssue::InvalidCharacters:
        return Str::LicenseChars;
    default:
        return Str::LicenseCheck;
    }
}

Str describeAuthError(AuthError error)
{
    switch (error)
    {
    case AuthError::InvalidCredentials:
        return Str::AuthInvalid;
    case AuthError::AccountLocked:
        return Str::AuthLocked;
    case AuthError::NetworkUnavailable:
        return Str::AuthNetwork;
    case AuthError::InvalidLicense:
        return Str::AuthLicenseInvalid;
    case AuthError::UsernameTaken:
        return Str::AuthUsernameTaken;
    default:
        return Str::AuthGeneric;
    }
}

}
