#pragma once

#include "text/shaper.hxx"

#include <cstddef>
#include <vector>

namespace loader::text
{

std::vector<float> caretOffsets(const ShapedText& shaped, std::size_t textLength);

std::size_t caretIndexAt(const std::vector<float>& offsets, float x);

}
