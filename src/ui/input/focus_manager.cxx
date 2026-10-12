#include "ui/input/focus_manager.hxx"

#include <algorithm>

namespace loader::ui::input
{

void FocusManager::setRoot(Widget* root)
{
    root_ = root;
    focused_ = nullptr;
}

void FocusManager::focus(Widget* widget, bool focusVisible)
{
    if (focused_ != nullptr && focused_ != widget)
        focused_->setFocused(false, false);

    focused_ = widget;
    if (focused_ != nullptr)
        focused_->setFocused(true, focusVisible);
}

void FocusManager::clear()
{
    focus(nullptr, false);
}

void FocusManager::forget(const Widget* widget)
{
    if (focused_ == widget)
        focused_ = nullptr;
    if (root_ == widget)
        root_ = nullptr;
}

void FocusManager::collect(Widget& widget, std::vector<Widget*>& order) const
{
    if (!widget.visible())
        return;
    if (widget.focusable())
        order.push_back(&widget);
    for (const auto& child : widget.children())
        collect(*child, order);
}

void FocusManager::advance(bool backwards)
{
    if (root_ == nullptr)
        return;

    std::vector<Widget*> order;
    collect(*root_, order);
    if (order.empty())
        return;

    const auto current = std::ranges::find(order, focused_);
    std::size_t index = 0;
    if (current == order.end())
        index = backwards ? order.size() - 1 : 0;
    else if (backwards)
        index = (static_cast<std::size_t>(current - order.begin()) + order.size() - 1) % order.size();
    else
        index = (static_cast<std::size_t>(current - order.begin()) + 1) % order.size();

    focus(order[index], true);
}

}
