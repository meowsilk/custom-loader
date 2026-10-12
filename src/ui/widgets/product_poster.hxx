#pragma once

#include "render/art_images.hxx"
#include "ui/widget.hxx"

namespace loader::ui
{

class ProductPoster : public Widget
{
public:
    ProductPoster(UiContext& context, render::ArtImage image, float width);

protected:
    void paintSelf(PaintContext& context) override;

private:
    render::ArtImage image_;
};

}
