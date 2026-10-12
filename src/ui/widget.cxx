#include "ui/widget.hxx"

#include <algorithm>
#include <ranges>

namespace loader::ui
{
namespace
{

std::vector<layout::FlexItem> collectItems(const std::vector<std::unique_ptr<Widget>>& children,
                                           std::vector<Widget*>& visibleChildren)
{
    std::vector<layout::FlexItem> items;
    for (const auto& child : children)
    {
        if (!child->visible())
            continue;
        visibleChildren.push_back(child.get());
        items.push_back({child->preferredSize(), child->style()});
    }
    return items;
}

}

Widget::Widget(UiContext& context) : context_(context) {}

Widget::~Widget()
{
    context_.notifyDestroyed(this);
}

Widget& Widget::attach(std::unique_ptr<Widget> child)
{
    child->parent_ = this;
    children_.push_back(std::move(child));
    invalidate();
    return *children_.back();
}

std::unique_ptr<Widget> Widget::detach(Widget& child)
{
    const auto found =
        std::ranges::find_if(children_, [&](const auto& item) { return item.get() == &child; });
    if (found == children_.end())
        return nullptr;

    std::unique_ptr<Widget> owned = std::move(*found);
    children_.erase(found);
    owned->parent_ = nullptr;
    invalidate();
    return owned;
}

core::Size Widget::preferredSize()
{
    core::Size content = measureContent();
    if (!children_.empty() && measuresChildren())
    {
        std::vector<Widget*> visibleChildren;
        const std::vector<layout::FlexItem> items = collectItems(children_, visibleChildren);
        const core::Size flex = layout::measureFlex(style_, items);
        content = {std::max(content.width, flex.width), std::max(content.height, flex.height)};
    }

    const float width = style_.width.value_or(content.width + style_.padding.horizontal());
    const float height = style_.height.value_or(content.height + style_.padding.vertical());
    return {width, height};
}

void Widget::layout(core::Rect bounds)
{
    bounds_ = bounds;
    layoutChildren(bounds);
}

void Widget::layoutChildren(core::Rect bounds)
{
    if (children_.empty())
        return;

    std::vector<Widget*> visibleChildren;
    const std::vector<layout::FlexItem> items = collectItems(children_, visibleChildren);
    const std::vector<core::Rect> rects = layout::arrangeFlex(style_, bounds, items);
    for (std::size_t i = 0; i < visibleChildren.size(); ++i)
        visibleChildren[i]->layout(rects[i]);
}

void Widget::paint(PaintContext& context)
{
    if (!visible_)
        return;

    const bool transformed = presentationOpacity_ < 1.0F || presentationOffset_ != core::Vec2{};
    if (transformed)
        context.draw().pushTransform(presentationOffset_, presentationOpacity_);

    paintSelf(context);

    const bool glass = glassSurface();
    const bool clip = clipsChildren();
    if (glass)
        context.enterGlass();
    if (clip)
        context.draw().pushClip(bounds_, 0.0F);
    for (const auto& child : children_)
        child->paint(context);
    if (clip)
        context.draw().popClip();
    if (glass)
        context.leaveGlass();

    if (transformed)
        context.draw().popTransform();
}

bool Widget::update(float deltaSeconds)
{
    bool animating = updateSelf(deltaSeconds);
    for (const auto& child : children_)
        animating = child->update(deltaSeconds) || animating;
    return animating;
}

std::optional<float> Widget::wakeDelay() const
{
    if (!visible_)
        return std::nullopt;

    std::optional<float> delay = idleWakeSeconds();
    for (const auto& child : children_)
    {
        if (const std::optional<float> childDelay = child->wakeDelay())
            delay = delay ? std::min(*delay, *childDelay) : *childDelay;
    }
    return delay;
}

Widget* Widget::hitTest(core::Vec2 point)
{
    if (!visible_ || !bounds_.contains(point))
        return nullptr;

    for (const auto& child : std::views::reverse(children_))
    {
        if (Widget* hit = child->hitTest(point))
            return hit;
    }
    return hitTestable() ? this : nullptr;
}

void Widget::setPresentation(core::Vec2 offset, float opacity)
{
    if (presentationOffset_ == offset && presentationOpacity_ == opacity)
        return;
    presentationOffset_ = offset;
    presentationOpacity_ = opacity;
    invalidate();
}

void Widget::setVisible(bool visible)
{
    if (visible_ == visible)
        return;
    visible_ = visible;
    invalidate();
}

void Widget::setEnabled(bool enabled)
{
    if (enabled_ == enabled)
        return;
    enabled_ = enabled;
    onStateChanged();
    invalidate();
}

void Widget::setHovered(bool hovered)
{
    if (hovered_ == hovered)
        return;
    hovered_ = hovered;
    onStateChanged();
    invalidate();
}

void Widget::setPressed(bool pressed)
{
    if (pressed_ == pressed)
        return;
    pressed_ = pressed;
    onStateChanged();
    invalidate();
}

void Widget::setFocused(bool focused, bool focusVisible)
{
    if (focused_ == focused && focusVisible_ == focusVisible)
        return;
    focused_ = focused;
    focusVisible_ = focusVisible;
    onStateChanged();
    invalidate();
}

}
