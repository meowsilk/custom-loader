#pragma once

#include "screens/screen.hxx"
#include "ui/paint_context.hxx"

#include <array>

namespace loader::screens
{

enum class WarmScreen
{
    Login,
    Register,
    Welcome,
    Main,
    Product,
};

inline constexpr std::array kWarmScreens = {
    WarmScreen::Login, WarmScreen::Register, WarmScreen::Welcome, WarmScreen::Main, WarmScreen::Product};

void warmUp(ui::UiContext& context, const Services& services, ui::PaintContext& paint, WarmScreen screen);

}
