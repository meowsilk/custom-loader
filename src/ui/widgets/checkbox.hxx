#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"

#include <functional>
#include <string>

namespace loader::ui
{

class Checkbox : public Widget
{
public:
    Checkbox(UiContext& context, const std::u32string& label);
    Checkbox(UiContext& context, Str id);

    void setLabel(const std::u32string& label);

    bool checked() const { return checked_; }
    void setChecked(bool checked);
    void setOnChanged(std::function<void(bool)> handler) { onChanged_ = std::move(handler); }

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
    void toggle();

    text::ShapedText label_;
    text::ShapedText tick_;
    TextBinding binding_;
    std::function<void(bool)> onChanged_;
    animation::Animated hover_;
    animation::Animated focus_;
    animation::Animated check_;
    bool checked_ = false;
};

}
