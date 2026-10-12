#include "ui/widgets/panel_surface.hxx"

namespace loader::ui
{

PanelSurface::PanelSurface(UiContext& context) : Widget(context) {}

void PanelSurface::setBackdropAvailable(bool available)
{
    if (backdropAvailable_ == available)
        return;
    backdropAvailable_ = available;
    invalidate();
}

void PanelSurface::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const render::Corners corners = render::Corners::uniform(theme.radii.panel);

    context.useBack();
    context.draw().setWindowShape(box, theme.radii.panel);
    context.draw().addShadow({box,
                              theme.radii.panel,
                              {0.0F, theme.glass.shadowOffset},
                              theme.glass.shadowBlur,
                              theme.colors.shadow});

    const float opacity = backdropAvailable_ ? 0.9F : 1.0F;
    context.draw().addRect({box, corners, core::Color::fromHex(0x0B0C0E, backdropAvailable_ ? 0.8F : 1.0F)});
    context.draw().addHero({box,
                            corners,
                            theme.colors.heroTint,
                            theme.backdrop.seed,
                            theme.backdrop.brightness,
                            opacity,
                            theme.backdrop.fade,
                            theme.backdrop.patternScale});
    context.draw().addRect({box, corners, core::Color::transparent(), theme.colors.panelBorder, 1.0F});
}

}
