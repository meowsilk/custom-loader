#pragma once

#include "core/color.hxx"
#include "core/geometry.hxx"
#include "ui/paint_context.hxx"

namespace loader::ui
{

void paintSpinner(
    PaintContext& context, core::Vec2 center, float diameter, float phase, const core::Color& color);

}
