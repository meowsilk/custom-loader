#include "ui/widgets/glass_surface.hxx"

namespace loader::ui
{

GlassSurface::GlassSurface(UiContext& context, float cornerRadius)
    : Widget(context), cornerRadius_(cornerRadius)
{
}

void GlassSurface::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    context.useGlass();
    context.draw().addGlass({bounds(),
                             render::Corners::uniform(cornerRadius_),
                             theme.colors.glassTint,
                             theme.colors.glassRim,
                             theme.glass.bevel,
                             theme.glass.refraction,
                             theme.glass.dispersion,
                             theme.glass.specular,
                             theme.glass.frost});
}

}
