#include "ui/input/input_router.hxx"

namespace loader::ui::input
{

void InputRouter::setRoot(Widget* root)
{
    setHovered(nullptr);
    captured_ = nullptr;
    root_ = root;
    focus_.setRoot(root);
}

void InputRouter::observe(UiContext& context)
{
    observation_ = context.observeDestruction([this](const Widget* widget) { widgetDestroyed(widget); });
}

void InputRouter::widgetDestroyed(const Widget* widget)
{
    if (hovered_ == widget)
        hovered_ = nullptr;
    if (captured_ == widget)
        captured_ = nullptr;
    if (root_ == widget)
        root_ = nullptr;
    focus_.forget(widget);
}

void InputRouter::setHovered(Widget* widget)
{
    if (hovered_ == widget)
        return;
    if (hovered_ != nullptr)
        hovered_->setHovered(false);
    hovered_ = widget;
    if (hovered_ != nullptr)
        hovered_->setHovered(true);
}

void InputRouter::updateHover(core::Vec2 position)
{
    if (root_ == nullptr)
    {
        setHovered(nullptr);
        return;
    }

    Widget* target = root_->hitTest(position);
    if (captured_ != nullptr)
        target = target == captured_ ? target : nullptr;
    if (target != nullptr && !target->enabled())
        target = nullptr;
    setHovered(target);
}

void InputRouter::pointerMoved(core::Vec2 position)
{
    updateHover(position);
    if (captured_ != nullptr)
        captured_->onPointerMove(position);
    else if (hovered_ != nullptr)
        hovered_->onHoverMove(position);
}

void InputRouter::pointerLeft()
{
    if (captured_ == nullptr)
        setHovered(nullptr);
}

void InputRouter::pointerButton(core::input::PointerButton button, bool down, core::Vec2 position)
{
    if (root_ == nullptr)
        return;

    if (down)
    {
        Widget* target = root_->hitTest(position);
        if (target == nullptr || !target->enabled())
        {
            focus_.clear();
            return;
        }

        captured_ = target;
        target->setPressed(true);
        if (target->focusable())
            focus_.focus(target, false);
        target->onPointerDown(position, button);
        updateHover(position);
        return;
    }

    if (captured_ == nullptr)
        return;

    Widget* released = captured_;
    captured_ = nullptr;
    released->onPointerUp(position, button);
    released->setPressed(false);
    if (button == core::input::PointerButton::Left && root_->hitTest(position) == released)
        released->onClick();
    updateHover(position);
}

void InputRouter::wheel(core::Vec2 position, float notches)
{
    if (root_ == nullptr)
        return;
    Widget* target = root_->hitTest(position);
    if (target != nullptr && target->enabled())
        target->onScroll(notches);
}

bool InputRouter::keyDown(const core::input::KeyEvent& event)
{
    if (event.key == core::input::Key::Tab)
    {
        focus_.advance(event.modifiers.shift);
        return true;
    }

    const bool activation = event.key == core::input::Key::Enter || event.key == core::input::Key::Space;
    if (event.repeat && activation)
        return true;

    if (Widget* focused = focus_.focused(); focused != nullptr && focused->onKeyDown(event))
        return true;

    return unhandledKey_ && unhandledKey_(event);
}

void InputRouter::character(char32_t codePoint)
{
    if (Widget* focused = focus_.focused())
        focused->onChar(codePoint);
}

void InputRouter::windowActivationChanged(bool active)
{
    if (active)
        return;
    captured_ = nullptr;
    setHovered(nullptr);
    if (Widget* focused = focus_.focused())
        focused->setFocused(true, false);
}

}
