#include "platform/win32_error.hxx"

#include <format>

namespace loader::platform
{

SystemError::SystemError(std::string_view operation, HRESULT code)
    : std::runtime_error(std::format("{} failed (0x{:08X})", operation, static_cast<unsigned long>(code))),
      code_(code)
{
}

void throwIfFailed(HRESULT result, std::string_view operation)
{
    if (FAILED(result))
        throw SystemError(operation, result);
}

void throwLastError(std::string_view operation)
{
    throw SystemError(operation, HRESULT_FROM_WIN32(GetLastError()));
}

}
