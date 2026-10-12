#pragma once

#include "ui/widget.hxx"

namespace loader::ui
{

class GlassSurface : public Widget
{
public:
    GlassSurface(UiContext& context, float cornerRadius);

protected:
    void paintSelf(PaintContext& context) override;
    bool glassSurface() const override { return true; }

private:
    float cornerRadius_;
};

}
