#pragma once

#include "storage/app_paths.hxx"

#include <string_view>

namespace loader::storage
{

class Logger
{
public:
    explicit Logger(AppPaths paths);

    void info(std::string_view message) const;
    void error(std::string_view message) const;

private:
    void write(std::string_view level, std::string_view message) const;
    void pruneOldFiles() const;

    AppPaths paths_;
};

}
