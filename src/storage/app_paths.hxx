#pragma once

#include <filesystem>

namespace loader::storage
{

class AppPaths
{
public:
    AppPaths();
    explicit AppPaths(std::filesystem::path root) : root_(std::move(root)) {}

    const std::filesystem::path& root() const { return root_; }
    std::filesystem::path vaultFile() const { return root_ / "vault" / "credentials.bin"; }
    std::filesystem::path settingsFile() const { return root_ / "settings.ini"; }
    std::filesystem::path logDirectory() const { return root_ / "logs"; }

private:
    std::filesystem::path root_;
};

}
