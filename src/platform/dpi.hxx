#pragma once

#include <Windows.h>

namespace loader::platform
{

inline constexpr unsigned kBaseDpi = 96;

unsigned dpiForMonitor(HMONITOR monitor);
float scaleFromDpi(unsigned dpi);

}
