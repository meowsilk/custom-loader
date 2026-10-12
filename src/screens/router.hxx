#pragma once

#include "screens/screen.hxx"
#include "screens/shell.hxx"
#include "ui/input/input_router.hxx"

#include <functional>
#include <memory>

namespace loader::screens
{

class Router final : public Navigator
{
public:
    Router(ui::UiContext& context,
           Shell& shell,
           ui::input::InputRouter& input,
           auth::AuthService& auth,
           storage::CredentialVault& vault,
           auth::Scheduler& scheduler,
           WindowControls& controls);

    void start();
    void update();
    const Services& services() const { return services_; }
    bool inputBlocked() const { return shell_.transitioning(); }

    void showLogin(std::optional<LoginPrefill> prefill) override;
    void showRegister() override;
    void showWelcome(std::u32string username) override;
    void showMain(std::u32string username, std::size_t product = kDefaultProduct) override;
    void showProduct(std::u32string username, std::size_t product) override;

    bool handleUnhandledKey(const core::input::KeyEvent& event);

private:
    void present(std::unique_ptr<Screen> screen, TransitionKind kind, bool animate);

    ui::UiContext& context_;
    Shell& shell_;
    ui::input::InputRouter& input_;
    LicenseStore licenses_;
    Services services_;
    std::unique_ptr<Screen> current_;
    std::unique_ptr<Screen> previous_;
};

}
