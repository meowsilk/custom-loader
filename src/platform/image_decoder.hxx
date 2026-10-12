#pragma once

#include "core/geometry.hxx"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace loader::platform
{

struct DecodedImage
{
    core::SizeI size;
    std::vector<std::uint8_t> bgra;
};

DecodedImage decodeImage(std::span<const std::byte> encoded);

}
