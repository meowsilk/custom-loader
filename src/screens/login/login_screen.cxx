#include "screens/login/login_screen.hxx"

#include "auth/validation.hxx"
#include "screens/messages.hxx"
#include "ui/icons.hxx"
#include "ui/widgets/label.hxx"
#include "ui/widgets/stack.hxx"

namespace loader::screens
{
namespace
{

using ui::layout::Align;
using ui::layout::Axis;
using ui::layout::Justify;

constexpr float kTopSpace = 1.0F;
constexpr float kBottomSpace = 1.5F;

}

LoginScreen::LoginScreen(ui::UiContext& context,
                         const Services& services,
                         std::optional<LoginPrefill> prefill)
    : services_(services), prefill_(std::move(prefill))
{
    buildLayout(context);
}

LoginScreen::~LoginScreen()
{
    storedSecret_.wipe();
    retainedPassword_.wipe();
}

void LoginScreen::buildLayout(ui::UiContext& context)
{
    const ui::theme::Theme& theme = context.theme();

    auto root = std::make_unique<ui::Stack>(context, Axis::Column);

    root->add<ui::Widget>().style().flexGrow = kTopSpace;

    auto& titleBlock = root->add<ui::Stack>(Axis::Column, 4.0F);
    titleBlock.add<ui::Label>(
        ui::Str::LoginTitle, theme.type.title, ui::TextTone::Primary, ui::HorizontalAlign::Center);
    titleBlock.add<ui::Label>(
        ui::Str::LoginSubtitle, theme.type.body, ui::TextTone::Secondary, ui::HorizontalAlign::Center);

    auto& form = root->add<ui::Stack>(Axis::Column, theme.space.m, core::Insets{16.0F, 28.0F, 16.0F, 0.0F});
    username_ = &form.add<ui::TextField>(ui::TextField::Kind::Plain, ui::icons::kUser, std::u32string());
    username_->bindPlaceholder(ui::Str::Username);
    password_ =
        &form.add<ui::TextField>(ui::TextField::Kind::Secret, ui::icons::kLockPassword, std::u32string());
    password_->bindPlaceholder(ui::Str::Password);

    auto& options = form.add<ui::Stack>(Axis::Row, 0.0F, core::Insets{2.0F, 0.0F, 2.0F, 0.0F}, Align::Center);
    options.style().justify = Justify::SpaceBetween;
    remember_ = &options.add<ui::Checkbox>(ui::Str::RememberMe);
    createAccount_ = &options.add<ui::Link>(ui::Str::CreateAccount);

    message_ = &form.add<ui::InlineMessage>();
    signIn_ = &form.add<ui::Button>(ui::Str::SignIn, ui::Button::Variant::Primary);

    root->add<ui::Widget>().style().flexGrow = kBottomSpace;

    username_->setOnChanged([this] { clearFeedback(); });
    password_->setOnChanged([this] { clearFeedback(); });
    username_->setOnSubmit([this] { submit(); });
    password_->setOnSubmit([this] { submit(); });
    signIn_->setOnClick([this] { submit(); });
    createAccount_->setOnClick([this] { services_.navigator.showRegister(); });

    content_ = std::move(root);
}

ui::Widget* LoginScreen::initialFocus()
{
    return username_->empty() ? static_cast<ui::Widget*>(username_) : static_cast<ui::Widget*>(password_);
}

void LoginScreen::restoreSavedCredentials()
{
    std::optional<storage::StoredCredentials> stored = services_.vault.load();
    if (!stored)
        return;

    username_->setText(stored->username);
    password_->setSavedSecret(true, &storedSecret_);
    storedSecret_ = std::move(stored->secret);
    remember_->setChecked(true);
}

void LoginScreen::onEnter()
{
    if (prefill_)
    {
        username_->setText(prefill_->username);
        if (prefill_->accountCreated)
            message_->show(ui::Str::AccountCreated, ui::TextTone::Success);
        return;
    }
    restoreSavedCredentials();
}

void LoginScreen::clearFeedback()
{
    username_->setError(false);
    password_->setError(false);
    message_->clear();
}

void LoginScreen::showFieldIssue(ui::TextField& field, ui::Str message)
{
    field.setError(true);
    message_->show(message, ui::TextTone::Danger);
}

void LoginScreen::setBusy(bool busy)
{
    busy_ = busy;
    username_->setReadOnly(busy);
    password_->setReadOnly(busy);
    remember_->setEnabled(!busy);
    createAccount_->setEnabled(!busy);
    signIn_->setLoading(busy);
}

void LoginScreen::cancelBusy()
{
    if (operation_)
        operation_->cancel();
    operation_.reset();
    retainedPassword_.wipe();
    setBusy(false);
}

bool LoginScreen::onUnhandledKey(const core::input::KeyEvent& event)
{
    if (event.key == core::input::Key::Escape && busy_)
    {
        cancelBusy();
        return true;
    }
    return false;
}

void LoginScreen::submit()
{
    if (busy_)
        return;

    clearFeedback();

    const auth::FieldIssue usernameIssue = auth::validateUsername(username_->text());
    if (usernameIssue != auth::FieldIssue::None)
    {
        showFieldIssue(*username_, describeUsernameIssue(usernameIssue));
        return;
    }

    if (!password_->hasSavedSecret())
    {
        const auth::FieldIssue passwordIssue = auth::validatePassword(password_->text());
        if (passwordIssue != auth::FieldIssue::None)
        {
            showFieldIssue(*password_, describePasswordIssue(passwordIssue));
            return;
        }
    }

    setBusy(true);
    startSignIn();
}

void LoginScreen::startSignIn()
{
    const bool usedSavedSecret = password_->hasSavedSecret();

    auth::SignInRequest request;
    request.username = std::u32string(username_->text());
    request.password.assign(usedSavedSecret ? storedSecret_.view() : password_->text());
    retainedPassword_.assign(request.password.view());

    operation_ = services_.auth.signIn(std::move(request),
                                       [this, usedSavedSecret](const auth::AuthResult& result)
                                       { completeSignIn(result, usedSavedSecret); });
}

void LoginScreen::completeSignIn(const auth::AuthResult& result, bool usedSavedSecret)
{
    operation_.reset();

    if (result.success())
    {
        if (remember_->checked())
            services_.vault.save(username_->text(), retainedPassword_.view());
        else
            services_.vault.remove();

        const std::u32string username(username_->text());
        retainedPassword_.wipe();
        storedSecret_.wipe();
        password_->clear();
        services_.navigator.showWelcome(username);
        return;
    }

    retainedPassword_.wipe();
    setBusy(false);
    password_->clear();

    if (usedSavedSecret)
    {
        services_.vault.remove();
        storedSecret_.wipe();
        remember_->setChecked(false);
    }

    const bool credentialsRejected = *result.error == auth::AuthError::InvalidCredentials;
    if (credentialsRejected)
        password_->setError(true);
    message_->show(describeAuthError(*result.error), ui::TextTone::Danger);
}

}
