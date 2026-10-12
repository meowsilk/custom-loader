#include "platform/dpi.hxx"

#include <ShellScalingApi.h>

namespace loader::platform
{
unsigned dpiForMonitor(HMONITOR monitor)
{
    UINT x = kBaseDpi;
    UINT y = kBaseDpi;
    if (monitor != nullptr && SUCCEEDED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y)))
        return x;
    return kBaseDpi;
}

float scaleFromDpi(unsigned dpi)
{
    return static_cast<float>(dpi) / static_cast<float>(kBaseDpi);
}

}
