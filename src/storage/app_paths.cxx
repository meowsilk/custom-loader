#include "storage/app_paths.hxx"

#include "platform/win32_error.hxx"

#include <ShlObj.h>

#include <Windows.h>

namespace loader::storage
{
namespace
{

constexpr wchar_t kApplicationFolder[] = L"loader.gg";

std::filesystem::path defaultRoot()
{
    PWSTR raw = nullptr;
    const HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &raw);
    const std::filesystem::path base =
        SUCCEEDED(result) ? std::filesystem::path(raw) : std::filesystem::path();
    CoTaskMemFree(raw);
    platform::throwIfFailed(result, "SHGetKnownFolderPath(LocalAppData)");
    return base / kApplicationFolder;
}

}

AppPaths::AppPaths() : root_(defaultRoot()) {}

}
