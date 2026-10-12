#pragma once

#include "screens/register/license_key.hxx"
#include "screens/screen.hxx"
#include "ui/widgets/button.hxx"
#include "ui/widgets/info_row.hxx"
#include "ui/widgets/inline_message.hxx"
#include "ui/widgets/revealer.hxx"
#include "ui/widgets/text_field.hxx"

#include <memory>
#include <string>

namespace loader::screens
{

ui::Str durationText(auth::LicenseDuration duration);

class ProductScreen final : public Screen
{
public:
    ProductScreen(ui::UiContext& context,
                  const Services& services,
                  std::u32string username,
                  std::size_t product,
                  const std::u32string& lastUpdate);
    ~ProductScreen() override;

    core::Size panelSize() const override { return {980.0F, 580.0F}; }
    std::unique_ptr<ui::Widget> takeHeaderContent() override { return std::move(header_); }
    bool onUnhandledKey(const core::input::KeyEvent& event) override;
    ui::Widget* initialFocus() override;

    ui::TextField& keyField() { return *key_; }

private:
    void buildLayout(ui::UiContext& context, const std::u32string& lastUpdate);
    void showLicensed(auth::LicenseDuration duration, bool animate);
    void goBack();
    void load();
    void submit();
    void completeActivation(const auth::AuthResult& result);
    void setBusy(bool busy);

    const Services& services_;
    std::u32string username_;
    std::size_t product_;
    LicenseKeyTransform keyTransform_;

    std::unique_ptr<ui::Widget> header_;
    ui::TextField* key_ = nullptr;
    ui::Button* activate_ = nullptr;
    ui::Button* load_ = nullptr;
    ui::InlineMessage* message_ = nullptr;
    ui::Revealer* keyRow_ = nullptr;
    ui::Revealer* loadRow_ = nullptr;
    ui::Revealer* licenseReveal_ = nullptr;
    ui::InfoRow* licenseRow_ = nullptr;

    std::unique_ptr<auth::PendingOperation> operation_;
    bool busy_ = false;
    bool licensed_ = false;
};

}
