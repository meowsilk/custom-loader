#pragma once

#include "ui/widget.hxx"

namespace loader::ui
{

class Divider : public Widget
{
public:
    Divider(UiContext& context, float height) : Widget(context)
    {
        style().width = 1.0F;
        style().height = height;
    }

protected:
    void paintSelf(PaintContext& context) override
    {
        context.useFront();
        context.draw().addRect({bounds(), render::Corners{}, core::Color::fromHex(0xFFFFFF, 0.16F)});
    }
};

}
