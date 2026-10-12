#pragma once

#include "core/geometry.hxx"

#include <optional>

namespace loader::ui::layout
{

enum class Axis
{
    Row,
    Column,
};

enum class Align
{
    Start,
    Center,
    End,
    Stretch,
};

enum class Justify
{
    Start,
    Center,
    End,
    SpaceBetween,
};

struct LayoutStyle
{
    Axis axis = Axis::Column;
    Align alignItems = Align::Stretch;
    Justify justify = Justify::Start;
    float gap = 0.0F;
    core::Insets padding;
    core::Insets margin;
    std::optional<float> width;
    std::optional<float> height;
    float flexGrow = 0.0F;
};

}
