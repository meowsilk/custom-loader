#pragma once

#include "ui/layout/layout_style.hxx"
#include "ui/widget.hxx"

namespace loader::ui
{

class Stack : public Widget
{
public:
    Stack(UiContext& context,
          layout::Axis axis,
          float gap = 0.0F,
          core::Insets padding = {},
          layout::Align align = layout::Align::Stretch)
        : Widget(context)
    {
        style().axis = axis;
        style().gap = gap;
        style().padding = padding;
        style().alignItems = align;
    }
};

}
