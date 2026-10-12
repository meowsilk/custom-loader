#include "screens/product/product_screen.hxx"

#include "auth/validation.hxx"
#include "screens/header_bar.hxx"
#include "screens/messages.hxx"
#include "screens/products.hxx"
#include "ui/icons.hxx"
#include "ui/widgets/glass_surface.hxx"
#include "ui/widgets/label.hxx"
#include "ui/widgets/paragraph_label.hxx"
#include "ui/widgets/product_poster.hxx"
#include "ui/widgets/stack.hxx"

namespace loader::screens
{
namespace
{

using ui::layout::Align;
using ui::layout::Axis;

constexpr float kPosterWidth = 220.0F;
constexpr float kColumnGap = 28.0F;
constexpr float kDescriptionWidth = 376.0F;
constexpr float kButtonWidth = 128.0F;
constexpr float kInfoRadius = 18.0F;
constexpr float kRowGap = 26.0F;
constexpr float kTopSpace = 1.0F;
constexpr float kBottomSpace = 1.15F;

}

ui::Str durationText(auth::LicenseDuration duration)
{
    switch (duration)
    {
    case auth::LicenseDuration::OneDay:
        return ui::Str::DurationDay;
    case auth::LicenseDuration::OneWeek:
        return ui::Str::DurationWeek;
    case auth::LicenseDuration::Lifetime:
        break;
    }
    return ui::Str::DurationLifetime;
}

ProductScreen::ProductScreen(ui::UiContext& context,
                             const Services& services,
                             std::u32string username,
                             std::size_t product,
                             const std::u32string& lastUpdate)
    : services_(services), username_(std::move(username)), product_(product % products().size())
{
    const ProductInfo& info = products()[product_];

    HeaderOptions options;
    options.onBack = [this] { goBack(); };
    options.title = info.name;
    header_ = makeHeaderBar(context, services_, username_, options);

    buildLayout(context, lastUpdate);
}

ProductScreen::~ProductScreen() = default;

ui::Widget* ProductScreen::initialFocus()
{
    return licensed_ ? static_cast<ui::Widget*>(load_) : static_cast<ui::Widget*>(key_);
}

void ProductScreen::buildLayout(ui::UiContext& context, const std::u32string& lastUpdate)
{
    const ui::theme::Theme& theme = context.theme();
    const ProductInfo& info = products()[product_];

    auto root = std::make_unique<ui::Stack>(
        context, Axis::Column, 0.0F, core::Insets{16.0F, 0.0F, 16.0F, 16.0F}, Align::Center);
    root->add<ui::Widget>().style().flexGrow = kTopSpace;

    auto& row = root->add<ui::Stack>(Axis::Row, kColumnGap, core::Insets{}, Align::Start);
    row.add<ui::ProductPoster>(info.art, kPosterWidth);

    auto& middle = row.add<ui::Stack>(Axis::Column, 0.0F, core::Insets{}, Align::Start);
    middle.add<ui::Label>(std::u32string(info.name), theme.type.display, ui::TextTone::Primary);

    auto& description = middle.add<ui::Stack>(Axis::Column, 0.0F, core::Insets{0.0F, 12.0F, 0.0F, 0.0F});
    description.add<ui::ParagraphLabel>(
        info.description, theme.type.body, ui::TextTone::Secondary, kDescriptionWidth);

    keyRow_ = &middle.add<ui::Revealer>();
    keyRow_->style().axis = Axis::Row;
    keyRow_->style().alignItems = Align::Center;
    keyRow_->style().gap = theme.space.s;
    keyRow_->style().padding = core::Insets{0.0F, kRowGap, 0.0F, 0.0F};
    keyRow_->style().width = kDescriptionWidth;
    key_ = &keyRow_->add<ui::TextField>(ui::TextField::Kind::Plain, ui::icons::kKey, std::u32string());
    key_->bindPlaceholder(ui::Str::KeyPrompt);
    key_->setTransform(&keyTransform_);
    key_->style().flexGrow = 1.0F;
    activate_ = &keyRow_->add<ui::Button>(ui::Str::Activate, ui::Button::Variant::Primary);
    activate_->style().width = kButtonWidth;

    loadRow_ = &middle.add<ui::Revealer>();
    loadRow_->style().axis = Axis::Row;
    loadRow_->style().padding = core::Insets{0.0F, kRowGap, 0.0F, 0.0F};
    loadRow_->style().width = kDescriptionWidth;
    load_ = &loadRow_->add<ui::Button>(ui::Str::Load, ui::Button::Variant::Primary);
    load_->style().flexGrow = 1.0F;

    auto& feedback = middle.add<ui::Stack>(Axis::Column, 0.0F, core::Insets{2.0F, 10.0F, 0.0F, 0.0F});
    message_ = &feedback.add<ui::InlineMessage>();

    auto& card = row.add<ui::GlassSurface>(kInfoRadius);
    card.style().axis = Axis::Column;
    card.style().padding = core::Insets::uniform(20.0F);
    card.style().gap = 18.0F;
    card.add<ui::InfoRow>(ui::icons::kEqualizer,
                          ui::Str::InfoInjectionType,
                          info.injection == InjectionType::Internal ? ui::Str::InjectionInternal
                                                                    : ui::Str::InjectionExternal);
    card.add<ui::InfoRow>(ui::icons::kCalendar, ui::Str::InfoLastUpdate, lastUpdate);
    card.add<ui::InfoRow>(ui::icons::kShield, ui::Str::InfoLastDetection, ui::Str::DetectionNever);
    licenseReveal_ = &card.add<ui::Revealer>();
    licenseRow_ =
        &licenseReveal_->add<ui::InfoRow>(ui::icons::kTime, ui::Str::InfoLicense, ui::Str::DurationLifetime);

    root->add<ui::Widget>().style().flexGrow = kBottomSpace;

    key_->setOnChanged(
        [this]
        {
            key_->setError(false);
            message_->clear();
        });
    key_->setOnSubmit([this] { submit(); });
    activate_->setOnClick([this] { submit(); });
    load_->setOnClick([this] { load(); });

    keyRow_->setRevealed(true, false);
    if (const std::optional<auth::LicenseDuration> existing = services_.licenses.find(product_))
        showLicensed(*existing, false);

    content_ = std::move(root);
}

void ProductScreen::showLicensed(auth::LicenseDuration duration, bool animate)
{
    licensed_ = true;
    licenseRow_->setValue(durationText(duration));
    keyRow_->setRevealed(false, false);
    loadRow_->setRevealed(true, animate);
    licenseReveal_->setRevealed(true, animate);
}

void ProductScreen::goBack()
{
    if (!busy_)
        services_.navigator.showMain(username_, product_);
}

void ProductScreen::load()
{
    goBack();
}

void ProductScreen::setBusy(bool busy)
{
    busy_ = busy;
    key_->setReadOnly(busy);
    activate_->setLoading(busy);
}

bool ProductScreen::onUnhandledKey(const core::input::KeyEvent& event)
{
    if (event.key != core::input::Key::Escape)
        return false;

    if (busy_)
    {
        if (operation_)
            operation_->cancel();
        operation_.reset();
        setBusy(false);
        return true;
    }

    goBack();
    return true;
}

void ProductScreen::submit()
{
    if (busy_)
        return;

    if (licensed_)
    {
        load();
        return;
    }

    const std::u32string normalized = normalizeLicenseKey(key_->text());
    const auth::FieldIssue issue = auth::validateLicenseKey(normalized);
    if (issue != auth::FieldIssue::None)
    {
        key_->setError(true);
        message_->show(describeLicenseIssue(issue), ui::TextTone::Danger);
        return;
    }

    setBusy(true);

    auth::ActivationRequest request;
    request.product = std::u32string(products()[product_].name);
    request.licenseKey = normalized;
    operation_ = services_.auth.activateProduct(
        std::move(request), [this](const auth::AuthResult& result) { completeActivation(result); });
}

void ProductScreen::completeActivation(const auth::AuthResult& result)
{
    operation_.reset();
    setBusy(false);

    if (result.success())
    {
        const auth::LicenseDuration duration = result.duration.value_or(auth::LicenseDuration::Lifetime);
        services_.licenses.set(product_, duration);
        key_->setError(false);
        message_->show(ui::Str::KeyAccepted, ui::TextTone::Success);
        showLicensed(duration, true);
        return;
    }

    key_->setError(true);
    message_->show(describeAuthError(*result.error), ui::TextTone::Danger);
}

}
