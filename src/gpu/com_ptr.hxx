#pragma once

#include <wrl/client.h>

namespace loader::gpu
{

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

}
