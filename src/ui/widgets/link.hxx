#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"

#include <functional>
#include <string>

namespace loader::ui
{

class Link : public Widget
{
public:
    Link(UiContext& context, const std::u32string& text);
    Link(UiContext& context, Str id);

    void setText(const std::u32string& text);

    void setOnClick(std::function<void()> handler) { onClick_ = std::move(handler); }

    void onClick() override;
    bool onKeyDown(const core::input::KeyEvent& event) override;
    core::input::Cursor cursorKind() const override { return core::input::Cursor::Hand; }

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;
    bool hitTestable() const override { return true; }
    void onStateChanged() override;

private:
    text::ShapedText shaped_;
    TextBinding binding_;
    std::function<void()> onClick_;
    animation::Animated hover_;
    animation::Animated focus_;
};

}
