#pragma once

#include "render/draw_list.hxx"
#include "ui/ui_context.hxx"

namespace loader::ui
{

class PaintContext
{
public:
    PaintContext(render::DrawList& draw, UiContext& ui) : draw_(draw), ui_(ui) {}

    render::DrawList& draw() const { return draw_; }
    UiContext& ui() const { return ui_; }
    const theme::Theme& theme() const { return ui_.theme(); }

    render::Layer glassLayer() const
    {
        return glassDepth_ > 0 ? render::Layer::GlassOverGlass : render::Layer::Glass;
    }
    void useBack() const { draw_.setLayer(render::Layer::Back); }
    void useGlass() const { draw_.setLayer(glassLayer()); }
    void useFront() const { draw_.setLayer(render::Layer::Front); }
    void useOverlayGlass() const { draw_.setLayer(render::Layer::OverlayGlass); }
    void useOverlayFront() const { draw_.setLayer(render::Layer::OverlayFront); }
    void enterGlass() { ++glassDepth_; }
    void leaveGlass() { --glassDepth_; }

private:
    render::DrawList& draw_;
    UiContext& ui_;
    int glassDepth_ = 0;
};

}
