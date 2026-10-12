#pragma once

#include "core/geometry.hxx"
#include "core/input.hxx"
#include "ui/input/focus_manager.hxx"
#include "ui/widget.hxx"

#include <functional>

namespace loader::ui::input
{

class InputRouter
{
public:
    using UnhandledKeyHandler = std::function<bool(const core::input::KeyEvent&)>;

    void setRoot(Widget* root);
    void observe(UiContext& context);
    void setUnhandledKeyHandler(UnhandledKeyHandler handler) { unhandledKey_ = std::move(handler); }

    FocusManager& focus() { return focus_; }
    bool captured() const { return captured_ != nullptr; }

    void pointerMoved(core::Vec2 position);
    void pointerLeft();
    void pointerButton(core::input::PointerButton button, bool down, core::Vec2 position);
    void wheel(core::Vec2 position, float notches);
    bool keyDown(const core::input::KeyEvent& event);
    void character(char32_t codePoint);
    void windowActivationChanged(bool active);

private:
    void widgetDestroyed(const Widget* widget);
    void updateHover(core::Vec2 position);
    void setHovered(Widget* widget);

    Widget* root_ = nullptr;
    Widget* hovered_ = nullptr;
    Widget* captured_ = nullptr;
    FocusManager focus_;
    UnhandledKeyHandler unhandledKey_;
    UiContext::Observation observation_;
};

}
