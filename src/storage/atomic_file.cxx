#include "storage/atomic_file.hxx"

#include "platform/win32_error.hxx"

#include <Windows.h>

#include <fstream>
#include <system_error>

namespace loader::storage
{

void writeFileAtomically(const std::filesystem::path& path, std::span<const std::byte> bytes)
{
    std::filesystem::create_directories(path.parent_path());

    std::filesystem::path temporary = path;
    temporary += L".tmp";

    {
        std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
        if (!file)
            throw platform::SystemError("open temporary file", HRESULT_FROM_WIN32(ERROR_WRITE_FAULT));
        file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        file.flush();
        if (!file)
            throw platform::SystemError("write temporary file", HRESULT_FROM_WIN32(ERROR_WRITE_FAULT));
    }

    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        const DWORD error = GetLastError();
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw platform::SystemError("replace file", HRESULT_FROM_WIN32(error));
    }
}

std::optional<std::vector<std::byte>> readFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        return std::nullopt;

    const std::streamsize size = file.tellg();
    if (size < 0)
        return std::nullopt;
    file.seekg(0);

    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    file.read(reinterpret_cast<char*>(bytes.data()), size);
    if (!file)
        return std::nullopt;
    return bytes;
}

void removeFileIfPresent(const std::filesystem::path& path)
{
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

}
