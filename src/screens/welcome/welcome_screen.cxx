#include "screens/welcome/welcome_screen.hxx"

#include "ui/widgets/label.hxx"
#include "ui/widgets/product_preview.hxx"
#include "ui/widgets/stack.hxx"

#include <chrono>

namespace loader::screens
{
namespace
{

using ui::layout::Align;
using ui::layout::Axis;

constexpr std::size_t kMaxNameLength = 16;
constexpr float kTopSpace = 1.0F;
constexpr float kBottomSpace = 1.3F;

std::u32string displayName(const std::u32string& username)
{
    if (username.size() <= kMaxNameLength)
        return username;
    return username.substr(0, kMaxNameLength - 1) + U"…";
}

}

WelcomeScreen::WelcomeScreen(ui::UiContext& context, const Services& services, std::u32string username)
    : services_(services), username_(std::move(username))
{
    buildLayout(context);
}

WelcomeScreen::~WelcomeScreen()
{
    if (handOff_)
        services_.scheduler.cancel(*handOff_);
}

void WelcomeScreen::buildLayout(ui::UiContext& context)
{
    const ui::theme::Theme& theme = context.theme();

    auto root = std::make_unique<ui::Stack>(
        context, Axis::Column, 0.0F, core::Insets{24.0F, 0.0F, 24.0F, 24.0F}, Align::Center);

    root->add<ui::Widget>().style().flexGrow = kTopSpace;
    root->add<ui::ProductPreview>();

    auto& text =
        root->add<ui::Stack>(Axis::Column, 6.0F, core::Insets{0.0F, 18.0F, 0.0F, 0.0F}, Align::Center);
    text.add<ui::Label>(ui::Str::WelcomeGreeting, theme.type.body, ui::TextTone::Secondary);
    text.add<ui::Label>(displayName(username_), theme.type.display, ui::TextTone::Primary);
    text.add<ui::Label>(ui::Str::WelcomeTagline, theme.type.body, ui::TextTone::Secondary);

    auto& progress =
        root->add<ui::Stack>(Axis::Column, 0.0F, core::Insets{0.0F, 26.0F, 0.0F, 0.0F}, Align::Center);
    bar_ = &progress.add<ui::LoadingBar>();
    bar_->setOnComplete([this] { scheduleHandOff(); });

    root->add<ui::Widget>().style().flexGrow = kBottomSpace;

    content_ = std::move(root);
}

void WelcomeScreen::scheduleHandOff()
{
    handOff_ = services_.scheduler.post(std::chrono::milliseconds(0),
                                        [this]
                                        {
                                            handOff_.reset();
                                            services_.navigator.showMain(username_);
                                        });
}

}
