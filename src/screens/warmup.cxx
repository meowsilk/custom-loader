#include "screens/warmup.hxx"

#include "platform/local_date.hxx"
#include "screens/login/login_screen.hxx"
#include "screens/main/main_screen.hxx"
#include "screens/product/product_screen.hxx"
#include "screens/register/register_screen.hxx"
#include "screens/shell.hxx"
#include "screens/welcome/welcome_screen.hxx"

#include <array>
#include <memory>

namespace loader::screens
{
namespace
{

constexpr core::Size kCarouselSize{932.0F, 500.0F};
constexpr std::u32string_view kSampleUser = U"user";
constexpr std::u32string_view kSampleKey = U"ABCD";
constexpr std::array kProgressSteps = {
    0.0F, 0.11F, 0.22F, 0.33F, 0.44F, 0.55F, 0.66F, 0.77F, 0.88F, 0.99F, 1.0F};

void paintScreen(Screen& screen, ui::PaintContext& paint)
{
    const core::Size size = screen.panelSize();
    const std::unique_ptr<ui::Widget> content = screen.takeContent();
    content->layout({0.0F, 0.0F, size.width, size.height});
    content->paint(paint);
    if (const std::unique_ptr<ui::Widget> header = screen.takeHeaderContent())
    {
        header->layout({0.0F, 0.0F, size.width, Shell::kCustomHeaderHeight});
        header->paint(paint);
    }
}

void warmWelcome(WelcomeScreen& welcome, ui::PaintContext& paint)
{
    const core::Size size = welcome.panelSize();
    const std::unique_ptr<ui::Widget> content = welcome.takeContent();
    content->layout({0.0F, 0.0F, size.width, size.height});
    for (const float progress : kProgressSteps)
    {
        welcome.bar().seek(progress);
        content->paint(paint);
    }
}

void warmMain(MainScreen& main, ui::PaintContext& paint)
{
    ui::ProductCarousel& carousel = main.carousel();
    const core::Rect bounds{0.0F, 0.0F, kCarouselSize.width, kCarouselSize.height};
    carousel.layout(bounds);
    for (std::size_t i = 0; i < carousel.productCount(); ++i)
    {
        carousel.select(i);
        carousel.update(1.0F);
        carousel.layout(bounds);
        carousel.paint(paint);
    }
    paintScreen(main, paint);
}

}

void warmUp(ui::UiContext& context, const Services& services, ui::PaintContext& paint, WarmScreen screen)
{
    const std::u32string user(kSampleUser);

    switch (screen)
    {
    case WarmScreen::Login:
    {
        LoginScreen login(context, services, std::nullopt);
        paintScreen(login, paint);
        break;
    }
    case WarmScreen::Register:
    {
        RegisterScreen registration(context, services);
        paintScreen(registration, paint);
        break;
    }
    case WarmScreen::Welcome:
    {
        WelcomeScreen welcome(context, services, user);
        warmWelcome(welcome, paint);
        break;
    }
    case WarmScreen::Main:
    {
        MainScreen main(context, services, user);
        warmMain(main, paint);
        break;
    }
    case WarmScreen::Product:
    {
        ProductScreen product(context, services, user, 0, platform::localDateText());
        product.keyField().setText(std::u32string(kSampleKey));
        paintScreen(product, paint);
        break;
    }
    }
}

}
