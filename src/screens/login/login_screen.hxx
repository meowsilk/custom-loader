#pragma once

#include "core/secret_string.hxx"
#include "screens/screen.hxx"
#include "ui/widgets/button.hxx"
#include "ui/widgets/checkbox.hxx"
#include "ui/widgets/inline_message.hxx"
#include "ui/widgets/link.hxx"
#include "ui/widgets/text_field.hxx"

#include <memory>
#include <optional>

namespace loader::screens
{

class LoginScreen final : public Screen
{
public:
    LoginScreen(ui::UiContext& context, const Services& services, std::optional<LoginPrefill> prefill);
    ~LoginScreen() override;

    core::Size panelSize() const override { return {420.0F, 540.0F}; }
    void onEnter() override;
    bool onUnhandledKey(const core::input::KeyEvent& event) override;
    ui::Widget* initialFocus() override;

private:
    void buildLayout(ui::UiContext& context);
    void restoreSavedCredentials();
    void clearFeedback();
    void showFieldIssue(ui::TextField& field, ui::Str message);
    void submit();
    void startSignIn();
    void completeSignIn(const auth::AuthResult& result, bool usedSavedSecret);
    void setBusy(bool busy);
    void cancelBusy();

    Services services_;
    std::optional<LoginPrefill> prefill_;

    ui::TextField* username_ = nullptr;
    ui::TextField* password_ = nullptr;
    ui::Checkbox* remember_ = nullptr;
    ui::InlineMessage* message_ = nullptr;
    ui::Button* signIn_ = nullptr;
    ui::Link* createAccount_ = nullptr;

    core::SecretString storedSecret_;
    core::SecretString retainedPassword_;
    std::unique_ptr<auth::PendingOperation> operation_;
    bool busy_ = false;
};

}
