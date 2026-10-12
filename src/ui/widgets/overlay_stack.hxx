#pragma once

#include "ui/widget.hxx"

#include <algorithm>
#include <utility>
#include <vector>

namespace loader::ui
{

class OverlayStack : public Widget
{
public:
    explicit OverlayStack(UiContext& context) : Widget(context) {}

    void pinSize(const Widget& child, core::Size size) { pinned_.emplace_back(&child, size); }
    void unpin() { pinned_.clear(); }

protected:
    bool clipsChildren() const override { return !pinned_.empty(); }
    bool measuresChildren() const override { return false; }

    void layoutChildren(core::Rect bounds) override
    {
        for (const auto& child : children())
        {
            const auto pin =
                std::ranges::find_if(pinned_, [&](const auto& entry) { return entry.first == child.get(); });
            if (pin == pinned_.end())
            {
                child->layout(bounds);
                continue;
            }

            const core::Vec2 center = bounds.center();
            child->layout({center.x - pin->second.width * 0.5F,
                           center.y - pin->second.height * 0.5F,
                           pin->second.width,
                           pin->second.height});
        }
    }

private:
    std::vector<std::pair<const Widget*, core::Size>> pinned_;
};

}
