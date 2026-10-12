#pragma once

#include "auth/validation.hxx"
#include "screens/screen.hxx"
#include "ui/widgets/button.hxx"
#include "ui/widgets/inline_message.hxx"
#include "ui/widgets/link.hxx"
#include "ui/widgets/text_field.hxx"

#include <array>
#include <memory>

namespace loader::screens
{

class RegisterScreen final : public Screen
{
public:
    RegisterScreen(ui::UiContext& context, const Services& services);
    ~RegisterScreen() override;

    core::Size panelSize() const override { return {420.0F, 552.0F}; }
    bool onUnhandledKey(const core::input::KeyEvent& event) override;
    ui::Widget* initialFocus() override { return username_; }

private:
    struct FieldState
    {
        ui::TextField* field = nullptr;
        bool touched = false;
    };

    void buildLayout(ui::UiContext& context);
    bool refreshValidation(bool revealAll);
    auth::FieldIssue issueFor(std::size_t index) const;
    ui::Str describe(std::size_t index, auth::FieldIssue issue) const;
    void submit();
    void completeRegistration(const auth::AuthResult& result);
    void setBusy(bool busy);

    Services services_;

    ui::TextField* username_ = nullptr;
    ui::TextField* password_ = nullptr;
    ui::TextField* confirmation_ = nullptr;
    ui::InlineMessage* message_ = nullptr;
    ui::Button* create_ = nullptr;
    ui::Link* back_ = nullptr;
    std::array<FieldState, 3> states_{};

    std::unique_ptr<auth::PendingOperation> operation_;
    bool busy_ = false;
};

}
