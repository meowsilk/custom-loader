#pragma once

#include "core/secret_string.hxx"

#include <optional>
#include <string>

namespace loader::auth
{

enum class AuthError
{
    InvalidCredentials,
    AccountLocked,
    NetworkUnavailable,
    InvalidLicense,
    UsernameTaken,
};

struct SignInRequest
{
    std::u32string username;
    core::SecretString password;
};

struct RegisterRequest
{
    std::u32string username;
    core::SecretString password;
};

struct ActivationRequest
{
    std::u32string product;
    std::u32string licenseKey;
};

enum class LicenseDuration
{
    OneDay,
    OneWeek,
    Lifetime,
};

struct AuthResult
{
    std::optional<AuthError> error;
    std::optional<LicenseDuration> duration;

    bool success() const { return !error.has_value(); }
    static AuthResult ok() { return {}; }
    static AuthResult failure(AuthError reason) { return {reason, std::nullopt}; }
    static AuthResult activated(LicenseDuration length) { return {std::nullopt, length}; }
};

}
