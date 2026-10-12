#pragma once

#include <Windows.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace loader::platform
{

class SystemError : public std::runtime_error
{
public:
    SystemError(std::string_view operation, HRESULT code);

    HRESULT code() const { return code_; }

private:
    HRESULT code_;
};

void throwIfFailed(HRESULT result, std::string_view operation);
[[noreturn]] void throwLastError(std::string_view operation);

}
