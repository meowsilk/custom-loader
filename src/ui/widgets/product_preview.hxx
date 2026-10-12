#pragma once

#include "ui/widget.hxx"

namespace loader::ui
{

class ProductPreview : public Widget
{
public:
    explicit ProductPreview(UiContext& context);

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;

private:
    float time_ = 0.0F;
};

}
