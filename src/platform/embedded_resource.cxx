#include "platform/embedded_resource.hxx"

#include "platform/win32_error.hxx"

#include <Windows.h>

namespace loader::platform
{

std::span<const std::byte> embeddedResource(const wchar_t* name)
{
    const HMODULE module = GetModuleHandleW(nullptr);

    const HRSRC resource = FindResourceW(module, name, RT_RCDATA);
    if (resource == nullptr)
        throwLastError("FindResource");

    const HGLOBAL loaded = LoadResource(module, resource);
    if (loaded == nullptr)
        throwLastError("LoadResource");

    const void* data = LockResource(loaded);
    const DWORD size = SizeofResource(module, resource);
    if (data == nullptr || size == 0)
        throw SystemError("LockResource", E_FAIL);

    return {static_cast<const std::byte*>(data), size};
}

}
