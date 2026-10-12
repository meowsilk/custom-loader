#pragma once

#include "ui/widgets/glass_surface.hxx"

namespace loader::ui
{

class TopBar : public GlassSurface
{
public:
    TopBar(UiContext& context, float cornerRadius);

protected:
    void layoutChildren(core::Rect bounds) override;
};

}
