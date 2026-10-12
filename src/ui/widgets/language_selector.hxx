#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"

#include <array>
#include <optional>

namespace loader::ui
{

class LanguageSelector : public Widget
{
public:
    explicit LanguageSelector(UiContext& context);

    bool open() const { return open_; }
    void setOpen(bool open);
    core::Rect chipRect() const;
    float expansion() const;
    core::Rect rowRect(std::size_t index) const;

    Widget* hitTest(core::Vec2 point) override;
    bool onPointerDown(core::Vec2 position, core::input::PointerButton button) override;
    void onPointerMove(core::Vec2 position) override;
    void onHoverMove(core::Vec2 position) override;
    void onPointerUp(core::Vec2 position, core::input::PointerButton button) override;
    bool onKeyDown(const core::input::KeyEvent& event) override;
    core::input::Cursor cursorKind() const override;

protected:
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;
    void onStateChanged() override;

private:
    core::Rect popupRect() const;
    std::optional<std::size_t> rowAt(core::Vec2 position) const;
    void choose(std::size_t index);
    void track(core::Vec2 position);
    std::size_t currentIndex() const;
    float collapsedWidth() const;
    float expandedWidth() const;

    std::array<text::ShapedText, kLanguageCount> names_;
    text::ShapedText chevronUp_;
    text::ShapedText chevronDown_;
    text::ShapedText check_;
    Localization::Subscription languageSubscription_;
    animation::Animated openAnimation_;
    animation::Animated chipHover_;
    animation::Animated focus_;
    bool open_ = false;
    bool pointerOnChip_ = false;
    bool pressedChip_ = false;
    std::optional<std::size_t> hoveredRow_;
    std::optional<std::size_t> pressedRow_;
    std::size_t highlighted_ = 0;
};

}
