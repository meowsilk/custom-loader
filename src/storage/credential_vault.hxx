#pragma once

#include "core/secret_string.hxx"
#include "storage/app_paths.hxx"

#include <optional>
#include <string>
#include <string_view>

namespace loader::storage
{

struct StoredCredentials
{
    std::u32string username;
    core::SecretString secret;
};

class CredentialVault
{
public:
    explicit CredentialVault(AppPaths paths) : paths_(std::move(paths)) {}

    bool exists() const;
    std::optional<StoredCredentials> load() const;
    bool save(std::u32string_view username, std::u32string_view secret) const;
    void remove() const;

private:
    AppPaths paths_;
};

}
