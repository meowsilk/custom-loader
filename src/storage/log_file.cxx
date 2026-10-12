#include "storage/log_file.hxx"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <system_error>
#include <vector>

namespace loader::storage
{
namespace
{

constexpr std::size_t kRetainedFiles = 7;

std::tm localTime(std::time_t time)
{
    std::tm result{};
    localtime_s(&result, &time);
    return result;
}

}

Logger::Logger(AppPaths paths) : paths_(std::move(paths)) {}

void Logger::pruneOldFiles() const
{
    std::error_code error;
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(paths_.logDirectory(), error))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".log")
            files.push_back(entry.path());
    }

    std::ranges::sort(files);
    while (files.size() > kRetainedFiles)
    {
        std::filesystem::remove(files.front(), error);
        files.erase(files.begin());
    }
}

void Logger::write(std::string_view level, std::string_view message) const
{
    std::error_code error;
    std::filesystem::create_directories(paths_.logDirectory(), error);
    if (error)
        return;

    const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::tm tm = localTime(now);
    const std::filesystem::path path =
        paths_.logDirectory() /
        std::format("loader-{:04}-{:02}-{:02}.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

    const bool isNewFile = !std::filesystem::exists(path);
    {
        std::ofstream file(path, std::ios::app);
        if (!file)
            return;
        file << std::format("{:02}:{:02}:{:02} [{}] {}\n", tm.tm_hour, tm.tm_min, tm.tm_sec, level, message);
    }
    if (isNewFile)
        pruneOldFiles();
}

void Logger::info(std::string_view message) const
{
    write("info", message);
}

void Logger::error(std::string_view message) const
{
    write("error", message);
}

}
