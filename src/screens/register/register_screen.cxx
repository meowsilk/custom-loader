#include "screens/register/register_screen.hxx"

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

constexpr float kTopSpace = 0.8F;
constexpr float kBottomSpace = 1.0F;

enum FieldIndex : std::size_t
{
    UsernameIndex = 0,
    PasswordIndex = 1,
    ConfirmationIndex = 2,
};

}

RegisterScreen::RegisterScreen(ui::UiContext& context, const Services& services) : services_(services)
{
    buildLayout(context);
}

RegisterScreen::~RegisterScreen() = default;

void RegisterScreen::buildLayout(ui::UiContext& context)
{
    const ui::theme::Theme& theme = context.theme();

    auto root = std::make_unique<ui::Stack>(context, Axis::Column);
    root->add<ui::Widget>().style().flexGrow = kTopSpace;

    auto& titleBlock = root->add<ui::Stack>(Axis::Column, 4.0F);
    titleBlock.add<ui::Label>(
        ui::Str::RegisterTitle, theme.type.title, ui::TextTone::Primary, ui::HorizontalAlign::Center);
    titleBlock.add<ui::Label>(
        ui::Str::RegisterSubtitle, theme.type.body, ui::TextTone::Secondary, ui::HorizontalAlign::Center);

    auto& form = root->add<ui::Stack>(Axis::Column, theme.space.m, core::Insets{16.0F, 24.0F, 16.0F, 0.0F});
    username_ = &form.add<ui::TextField>(ui::TextField::Kind::Plain, ui::icons::kUser, std::u32string());
    username_->bindPlaceholder(ui::Str::Username);
    password_ =
        &form.add<ui::TextField>(ui::TextField::Kind::Secret, ui::icons::kLockPassword, std::u32string());
    password_->bindPlaceholder(ui::Str::Password);
    confirmation_ =
        &form.add<ui::TextField>(ui::TextField::Kind::Secret, ui::icons::kLockPassword, std::u32string());
    confirmation_->bindPlaceholder(ui::Str::ConfirmPassword);

    message_ = &form.add<ui::InlineMessage>();
    create_ = &form.add<ui::Button>(ui::Str::CreateAccount, ui::Button::Variant::Primary);

    auto& footer =
        root->add<ui::Stack>(Axis::Row, 0.0F, core::Insets{0.0F, 16.0F, 0.0F, 0.0F}, Align::Center);
    footer.style().justify = Justify::Center;
    back_ = &footer.add<ui::Link>(ui::Str::BackToSignIn);

    root->add<ui::Widget>().style().flexGrow = kBottomSpace;

    states_ = {{{username_, false}, {password_, false}, {confirmation_, false}}};

    for (std::size_t i = 0; i < states_.size(); ++i)
    {
        ui::TextField& field = *states_[i].field;
        field.setOnChanged([this] { refreshValidation(false); });
        field.setOnBlur(
            [this, i]
            {
                states_[i].touched = true;
                refreshValidation(false);
            });
        field.setOnSubmit([this] { submit(); });
    }

    create_->setOnClick([this] { submit(); });
    back_->setOnClick([this] { services_.navigator.showLogin(std::nullopt); });

    content_ = std::move(root);
}

auth::FieldIssue RegisterScreen::issueFor(std::size_t index) const
{
    switch (index)
    {
    case UsernameIndex:
        return auth::validateUsername(username_->text());
    case PasswordIndex:
        return auth::validatePassword(password_->text());
    default:
        return auth::validatePasswordConfirmation(password_->text(), confirmation_->text());
    }
}

ui::Str RegisterScreen::describe(std::size_t index, auth::FieldIssue issue) const
{
    switch (index)
    {
    case UsernameIndex:
        return describeUsernameIssue(issue);
    case PasswordIndex:
        return describePasswordIssue(issue);
    default:
        return describeConfirmationIssue(issue);
    }
}

bool RegisterScreen::refreshValidation(bool revealAll)
{
    bool allValid = true;
    std::optional<ui::Str> firstMessage;

    for (std::size_t i = 0; i < states_.size(); ++i)
    {
        const auth::FieldIssue issue = issueFor(i);
        const bool valid = issue == auth::FieldIssue::None;
        allValid = allValid && valid;

        const bool show = !valid && (revealAll || states_[i].touched) &&
                          !(states_[i].field->focused() && issue == auth::FieldIssue::Empty);
        states_[i].field->setError(show);
        if (show && !firstMessage)
            firstMessage = describe(i, issue);
    }

    if (firstMessage)
        message_->show(*firstMessage, ui::TextTone::Danger);
    else
        message_->clear();
    return allValid;
}

void RegisterScreen::setBusy(bool busy)
{
    busy_ = busy;
    for (const FieldState& state : states_)
        state.field->setReadOnly(busy);
    back_->setEnabled(!busy);
    create_->setLoading(busy);
}

bool RegisterScreen::onUnhandledKey(const core::input::KeyEvent& event)
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

    services_.navigator.showLogin(std::nullopt);
    return true;
}

void RegisterScreen::submit()
{
    if (busy_)
        return;

    for (FieldState& state : states_)
        state.touched = true;
    if (!refreshValidation(true))
        return;

    setBusy(true);

    auth::RegisterRequest request;
    request.username = std::u32string(username_->text());
    request.password.assign(password_->text());

    operation_ = services_.auth.registerAccount(
        std::move(request), [this](const auth::AuthResult& result) { completeRegistration(result); });
}

void RegisterScreen::completeRegistration(const auth::AuthResult& result)
{
    operation_.reset();

    if (result.success())
    {
        LoginPrefill prefill;
        prefill.username = std::u32string(username_->text());
        prefill.accountCreated = true;
        password_->clear();
        confirmation_->clear();
        services_.navigator.showLogin(std::move(prefill));
        return;
    }

    setBusy(false);
    const bool usernameProblem = *result.error == auth::AuthError::UsernameTaken;
    username_->setError(usernameProblem);
    message_->show(describeAuthError(*result.error), ui::TextTone::Danger);
}

}
