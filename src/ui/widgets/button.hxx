#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"

#include <functional>
#include <string>

namespace loader::ui
{

class Button : public Widget
{
public:
    enum class Variant
    {
        Primary,
        Secondary,
    };

    Button(UiContext& context, std::u32string label, Variant variant = Variant::Primary);

    Button(UiContext& context, Str id, Variant variant = Variant::Primary);

    void setOnClick(std::function<void()> handler) { onClick_ = std::move(handler); }
    void setLabel(std::u32string label);
    void setLoading(bool loading);
    bool loading() const { return loading_; }

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
    std::u32string label_;
    Variant variant_;
    text::ShapedText shaped_;
    std::function<void()> onClick_;
    animation::Animated hover_;
    animation::Animated press_;
    animation::Animated focus_;
    animation::Animated busy_;
    TextBinding binding_;
    float spinnerPhase_ = 0.0F;
    bool loading_ = false;
};

}
