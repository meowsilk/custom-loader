#pragma once

#include "storage/app_paths.hxx"

#include <optional>
#include <string>
#include <string_view>

namespace loader::storage
{

class Settings
{
public:
    explicit Settings(AppPaths paths) : paths_(std::move(paths)) {}

    std::optional<std::string> language() const;
    bool setLanguage(std::string_view code) const;

private:
    AppPaths paths_;
};

}
