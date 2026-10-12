#pragma once

#include "core/input.hxx"

#include <Windows.h>

namespace loader::platform
{

core::input::Key keyFromVirtualKey(WPARAM virtualKey);
core::input::Modifiers currentModifiers();

}
