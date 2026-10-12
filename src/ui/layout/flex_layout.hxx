#pragma once

#include "core/geometry.hxx"
#include "ui/layout/layout_style.hxx"

#include <span>
#include <vector>

namespace loader::ui::layout
{

struct FlexItem
{
    core::Size preferred;
    LayoutStyle style;
};

core::Size measureFlex(const LayoutStyle& container, std::span<const FlexItem> items);

std::vector<core::Rect>
arrangeFlex(const LayoutStyle& container, core::Rect bounds, std::span<const FlexItem> items);

}
