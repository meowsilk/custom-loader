#include "ui/layout/flex_layout.hxx"

#include <algorithm>
#include <numeric>

namespace loader::ui::layout
{
namespace
{

struct AxisView
{
    bool horizontal;

    float main(core::Size size) const { return horizontal ? size.width : size.height; }
    float cross(core::Size size) const { return horizontal ? size.height : size.width; }
    float mainStart(const core::Insets& insets) const { return horizontal ? insets.left : insets.top; }
    float mainEnd(const core::Insets& insets) const { return horizontal ? insets.right : insets.bottom; }
    float crossStart(const core::Insets& insets) const { return horizontal ? insets.top : insets.left; }
    float crossEnd(const core::Insets& insets) const { return horizontal ? insets.bottom : insets.right; }
    float mainMargins(const core::Insets& insets) const { return mainStart(insets) + mainEnd(insets); }
    float crossMargins(const core::Insets& insets) const { return crossStart(insets) + crossEnd(insets); }
};

AxisView viewFor(const LayoutStyle& style)
{
    return AxisView{style.axis == Axis::Row};
}

}

core::Size measureFlex(const LayoutStyle& container, std::span<const FlexItem> items)
{
    const AxisView axis = viewFor(container);

    float main = 0.0F;
    float cross = 0.0F;
    for (const FlexItem& item : items)
    {
        main += axis.main(item.preferred) + axis.mainMargins(item.style.margin);
        cross = std::max(cross, axis.cross(item.preferred) + axis.crossMargins(item.style.margin));
    }
    if (!items.empty())
        main += container.gap * static_cast<float>(items.size() - 1);

    return axis.horizontal ? core::Size{main, cross} : core::Size{cross, main};
}

std::vector<core::Rect>
arrangeFlex(const LayoutStyle& container, core::Rect bounds, std::span<const FlexItem> items)
{
    const AxisView axis = viewFor(container);
    const core::Rect content = bounds.inset(container.padding);
    const float contentMain = axis.horizontal ? content.width : content.height;
    const float contentCross = axis.horizontal ? content.height : content.width;

    const core::Size measured = measureFlex(container, items);
    const float used = axis.main(measured);
    const float free = std::max(0.0F, contentMain - used);

    const float totalGrow =
        std::accumulate(items.begin(),
                        items.end(),
                        0.0F,
                        [](float sum, const FlexItem& item) { return sum + item.style.flexGrow; });

    float cursor = 0.0F;
    float spacing = container.gap;
    if (totalGrow <= 0.0F && !items.empty())
    {
        switch (container.justify)
        {
        case Justify::Start:
            break;
        case Justify::Center:
            cursor = free * 0.5F;
            break;
        case Justify::End:
            cursor = free;
            break;
        case Justify::SpaceBetween:
            if (items.size() > 1)
                spacing += free / static_cast<float>(items.size() - 1);
            break;
        }
    }

    std::vector<core::Rect> rects;
    rects.reserve(items.size());

    for (const FlexItem& item : items)
    {
        float mainSize = axis.main(item.preferred);
        if (totalGrow > 0.0F)
            mainSize += free * item.style.flexGrow / totalGrow;

        const float crossAvailable = contentCross - axis.crossMargins(item.style.margin);
        float crossSize = axis.cross(item.preferred);
        if (container.alignItems == Align::Stretch)
        {
            const bool fixedCross =
                axis.horizontal ? item.style.height.has_value() : item.style.width.has_value();
            if (!fixedCross)
                crossSize = std::max(0.0F, crossAvailable);
        }

        float crossOffset = 0.0F;
        switch (container.alignItems)
        {
        case Align::Start:
        case Align::Stretch:
            break;
        case Align::Center:
            crossOffset = (crossAvailable - crossSize) * 0.5F;
            break;
        case Align::End:
            crossOffset = crossAvailable - crossSize;
            break;
        }

        const float mainPosition = cursor + axis.mainStart(item.style.margin);
        const float crossPosition = crossOffset + axis.crossStart(item.style.margin);

        rects.push_back(
            axis.horizontal
                ? core::Rect{content.x + mainPosition, content.y + crossPosition, mainSize, crossSize}
                : core::Rect{content.x + crossPosition, content.y + mainPosition, crossSize, mainSize});

        cursor += mainSize + axis.mainMargins(item.style.margin) + spacing;
    }

    return rects;
}

}
