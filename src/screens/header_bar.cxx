#include "screens/header_bar.hxx"

#include "screens/shell.hxx"
#include "ui/icons.hxx"
#include "ui/widgets/divider.hxx"
#include "ui/widgets/label.hxx"
#include "ui/widgets/stack.hxx"
#include "ui/widgets/top_bar.hxx"

namespace loader::screens
{
namespace
{

using ui::layout::Align;
using ui::layout::Axis;

constexpr float kBarHeight = 40.0F;
constexpr float kBarRadius = 14.0F;
constexpr float kDividerHeight = 18.0F;
constexpr std::size_t kMaxNameLength = 18;

std::u32string displayName(const std::u32string& username)
{
    if (username.size() <= kMaxNameLength)
        return username;
    return username.substr(0, kMaxNameLength - 1) + U"…";
}

}

std::unique_ptr<ui::Widget> makeHeaderBar(ui::UiContext& context,
                                          const Services& services,
                                          const std::u32string& username,
                                          const HeaderOptions& options)
{
    const ui::theme::Theme& theme = context.theme();

    auto bar = std::make_unique<ui::TopBar>(context, kBarRadius);
    bar->style().height = kBarHeight;
    bar->style().padding = core::Insets{options.onBack ? 4.0F : 14.0F, 0.0F, 4.0F, 0.0F};

    auto& left = bar->add<ui::Stack>(Axis::Row, 6.0F, core::Insets{}, Align::Center);
    if (options.onBack)
    {
        auto& back = left.add<ui::IconButton>(ui::icons::kArrowLeft, ui::IconButton::Tone::Neutral);
        back.setOnClick(options.onBack);
    }
    left.add<ui::Label>(U"loader.gg", theme.type.brand, ui::TextTone::Primary);

    if (options.titleId)
        bar->add<ui::Label>(*options.titleId, theme.type.heading, ui::TextTone::Primary);
    else
        bar->add<ui::Label>(options.title, theme.type.heading, ui::TextTone::Primary);

    auto& right = bar->add<ui::Stack>(Axis::Row, 4.0F, core::Insets{}, Align::Center);
    right.add<ui::Label>(displayName(username), theme.type.label, ui::TextTone::Primary);
    auto& signOut = right.add<ui::IconButton>(ui::icons::kLogout, ui::IconButton::Tone::Neutral);
    signOut.setOnClick([&services] { services.navigator.showLogin(std::nullopt); });

    right.add<ui::Divider>(kDividerHeight);

    auto& minimize = right.add<ui::IconButton>(ui::icons::kMinimize, ui::IconButton::Tone::Neutral);
    minimize.setOnClick([&services] { services.controls.minimizeWindow(); });
    auto& close = right.add<ui::IconButton>(ui::icons::kClose, ui::IconButton::Tone::Danger);
    close.setOnClick([&services] { services.controls.closeWindow(); });

    return bar;
}

}
