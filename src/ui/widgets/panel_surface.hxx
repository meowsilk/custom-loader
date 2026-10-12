#pragma once

#include "ui/widget.hxx"

namespace loader::ui
{

class PanelSurface : public Widget
{
public:
    explicit PanelSurface(UiContext& context);

    void setBackdropAvailable(bool available);

protected:
    void paintSelf(PaintContext& context) override;

private:
    bool backdropAvailable_ = false;
};

}
