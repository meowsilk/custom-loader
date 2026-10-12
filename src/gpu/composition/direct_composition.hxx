#pragma once

#include "gpu/composition/composition.hxx"

#include <Windows.h>

#include <memory>

namespace loader::gpu
{

std::unique_ptr<Composition> createDirectComposition(HWND stageWindow);

}
