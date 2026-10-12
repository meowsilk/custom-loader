#include "ui/widgets/top_bar.hxx"

namespace loader::ui
{
namespace
{

constexpr float kStart = 0.0F;
constexpr float kCenter = 0.5F;
constexpr float kEnd = 1.0F;

}

TopBar::TopBar(UiContext& context, float cornerRadius) : GlassSurface(context, cornerRadius) {}

void TopBar::layoutChildren(core::Rect bounds)
{
    const std::span<const std::unique_ptr<Widget>> items = children();
    if (items.size() != 3)
    {
        Widget::layoutChildren(bounds);
        return;
    }

    const core::Rect inner = bounds.inset(style().padding);
    const auto place = [&](Widget& child, float anchor)
    {
        const core::Size size = child.preferredSize();
        const float x = inner.x + (inner.width - size.width) * anchor;
        child.layout({x, inner.y + (inner.height - size.height) * 0.5F, size.width, size.height});
    };
    place(*items[0], kStart);
    place(*items[1], kCenter);
    place(*items[2], kEnd);
}

}
