#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/widget.hxx"

#include <functional>

namespace loader::ui
{

class IconButton : public Widget
{
public:
    enum class Tone
    {
        Neutral,
        Danger,
    };

    IconButton(UiContext& context, char32_t glyph, Tone tone = Tone::Neutral);

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
    Tone tone_;
    std::function<void()> onClick_;
    animation::Animated hover_;
    animation::Animated press_;
    animation::Animated focus_;
};

}
