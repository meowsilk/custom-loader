#pragma once

#include "core/geometry.hxx"
#include "core/input.hxx"
#include "ui/layout/flex_layout.hxx"
#include "ui/layout/layout_style.hxx"
#include "ui/paint_context.hxx"
#include "ui/ui_context.hxx"

#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace loader::ui
{

class Widget
{
public:
    explicit Widget(UiContext& context);
    virtual ~Widget();

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    template<typename T, typename... Args>
    T& add(Args&&... args)
    {
        auto child = std::make_unique<T>(context_, std::forward<Args>(args)...);
        T& reference = *child;
        attach(std::move(child));
        return reference;
    }

    Widget& attach(std::unique_ptr<Widget> child);
    std::unique_ptr<Widget> detach(Widget& child);

    Widget* parent() const { return parent_; }
    std::span<const std::unique_ptr<Widget>> children() const { return children_; }

    layout::LayoutStyle& style() { return style_; }
    const layout::LayoutStyle& style() const { return style_; }

    core::Rect bounds() const { return bounds_; }
    core::Size preferredSize();
    void layout(core::Rect bounds);

    void paint(PaintContext& context);
    bool update(float deltaSeconds);
    virtual Widget* hitTest(core::Vec2 point);
    std::optional<float> wakeDelay() const;

    bool visible() const { return visible_; }
    void setVisible(bool visible);
    void setPresentation(core::Vec2 offset, float opacity);
    bool enabled() const { return enabled_; }
    void setEnabled(bool enabled);
    bool hovered() const { return hovered_; }
    bool pressed() const { return pressed_; }
    bool focused() const { return focused_; }
    bool focusVisible() const { return focused_ && focusVisible_; }
    bool focusable() const { return focusable_ && enabled_ && visible_; }

    void setHovered(bool hovered);
    void setPressed(bool pressed);
    void setFocused(bool focused, bool focusVisible);

    virtual bool onPointerDown(core::Vec2, core::input::PointerButton) { return false; }
    virtual void onPointerMove(core::Vec2) {}
    virtual void onPointerUp(core::Vec2, core::input::PointerButton) {}
    virtual void onHoverMove(core::Vec2) {}
    virtual void onClick() {}
    virtual void onScroll(float) {}
    virtual bool onKeyDown(const core::input::KeyEvent&) { return false; }
    virtual void onChar(char32_t) {}
    virtual core::input::Cursor cursorKind() const { return core::input::Cursor::Arrow; }

protected:
    virtual core::Size measureContent() { return {}; }
    virtual void layoutChildren(core::Rect bounds);
    virtual void paintSelf(PaintContext&) {}
    virtual bool updateSelf(float) { return false; }
    virtual std::optional<float> idleWakeSeconds() const { return std::nullopt; }
    virtual bool hitTestable() const { return false; }
    virtual bool glassSurface() const { return false; }
    virtual bool clipsChildren() const { return false; }
    virtual bool measuresChildren() const { return true; }
    virtual void onStateChanged() {}

    void setFocusable(bool focusable) { focusable_ = focusable; }
    void invalidate() const { context_.requestFrame(); }

    UiContext& context_;

private:
    Widget* parent_ = nullptr;
    std::vector<std::unique_ptr<Widget>> children_;
    layout::LayoutStyle style_;
    core::Rect bounds_;
    core::Vec2 presentationOffset_;
    float presentationOpacity_ = 1.0F;
    bool visible_ = true;
    bool enabled_ = true;
    bool hovered_ = false;
    bool pressed_ = false;
    bool focused_ = false;
    bool focusVisible_ = false;
    bool focusable_ = false;
};

}
