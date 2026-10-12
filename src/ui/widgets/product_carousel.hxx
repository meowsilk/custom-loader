#pragma once

#include "render/art_images.hxx"
#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/animation/spring.hxx"
#include "ui/widget.hxx"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace loader::ui
{

class ProductCarousel : public Widget
{
public:
    explicit ProductCarousel(UiContext& context);

    void addProduct(std::u32string title, render::ArtImage image);
    void setOnActivate(std::function<void(std::size_t)> handler) { onActivate_ = std::move(handler); }

    std::size_t selectedIndex() const { return selected_; }
    std::size_t productCount() const { return items_.size(); }
    const std::u32string& productTitle(std::size_t index) const { return items_[index].title; }
    core::Rect cardRect(std::size_t index) const;

    void select(std::size_t index);
    void jumpTo(std::size_t index);
    void rotate(int steps);

    bool onPointerDown(core::Vec2 position, core::input::PointerButton button) override;
    void onPointerMove(core::Vec2 position) override;
    void onHoverMove(core::Vec2 position) override;
    void onPointerUp(core::Vec2 position, core::input::PointerButton button) override;
    void onScroll(float notches) override;
    bool onKeyDown(const core::input::KeyEvent& event) override;
    core::input::Cursor cursorKind() const override;

protected:
    void layoutChildren(core::Rect bounds) override;
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;
    bool hitTestable() const override { return true; }
    void onStateChanged() override;

private:
    struct Item
    {
        std::u32string title;
        render::ArtImage image;
        text::ShapedText shaped;
        animation::Animated hover;
        animation::Animated press;
    };

    struct Placement
    {
        core::Rect rect;
        float depth = 0.0F;
        float opacity = 1.0F;
        float dim = 0.0F;
    };

    float frontHeight() const;
    float offsetOf(std::size_t index) const;
    Placement placementFor(std::size_t index) const;
    std::optional<std::size_t> cardAt(core::Vec2 position) const;
    std::optional<std::size_t> dotAt(core::Vec2 position) const;
    core::Rect dotSlot(std::size_t index) const;
    void trackPointer(core::Vec2 position);
    void activate();
    void fitTitles();
    void paintCard(PaintContext& context, std::size_t index, const Placement& placement);
    void paintArrow(PaintContext& context, const core::Rect& box, float hover, float frontness) const;
    void paintDots(PaintContext& context) const;

    std::vector<Item> items_;
    text::ShapedText arrow_;
    std::size_t selected_ = 0;
    animation::Spring position_{0.0F};
    animation::Animated focus_;
    std::optional<std::size_t> hovered_;
    std::optional<std::size_t> hoveredDot_;
    std::optional<std::size_t> pressedCard_;
    std::optional<std::size_t> pressedDot_;
    float fittedWidth_ = 0.0F;
    float scrollRemainder_ = 0.0F;
    std::function<void(std::size_t)> onActivate_;
};

}
