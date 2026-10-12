#pragma once

#include <cstddef>
#include <span>

namespace loader::platform
{

std::span<const std::byte> embeddedResource(const wchar_t* name);

}
