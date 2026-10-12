#include "screens/router.hxx"

#include "platform/local_date.hxx"
#include "screens/login/login_screen.hxx"
#include "screens/main/main_screen.hxx"
#include "screens/product/product_screen.hxx"
#include "screens/register/register_screen.hxx"
#include "screens/welcome/welcome_screen.hxx"

namespace loader::screens
{

Router::Router(ui::UiContext& context,
               Shell& shell,
               ui::input::InputRouter& input,
               auth::AuthService& auth,
               storage::CredentialVault& vault,
               auth::Scheduler& scheduler,
               WindowControls& controls)
    : context_(context), shell_(shell), input_(input),
      services_{auth, vault, scheduler, *this, controls, licenses_}
{
}

void Router::start()
{
    present(std::make_unique<LoginScreen>(context_, services_, std::nullopt),
            TransitionKind::SlideForward,
            false);
}

void Router::update()
{
    if (previous_ && !shell_.transitioning())
        previous_.reset();
}

void Router::present(std::unique_ptr<Screen> screen, TransitionKind kind, bool animate)
{
    if (previous_)
        previous_.reset();
    previous_ = std::move(current_);
    current_ = std::move(screen);

    const core::Size size = current_->panelSize();
    std::unique_ptr<ui::Widget> content = current_->takeContent();
    std::unique_ptr<ui::Widget> header = current_->takeHeaderContent();
    if (animate)
        shell_.transitionTo(std::move(content), size, kind, std::move(header));
    else
        shell_.showInitial(std::move(content), size, std::move(header));

    input_.setRoot(&shell_);
    current_->onEnter();
    if (ui::Widget* focus = current_->initialFocus())
        input_.focus().focus(focus, false);
}

void Router::showLogin(std::optional<LoginPrefill> prefill)
{
    present(std::make_unique<LoginScreen>(context_, services_, std::move(prefill)),
            TransitionKind::SlideBackward,
            true);
}

void Router::showRegister()
{
    present(std::make_unique<RegisterScreen>(context_, services_), TransitionKind::SlideForward, true);
}

void Router::showWelcome(std::u32string username)
{
    present(std::make_unique<WelcomeScreen>(context_, services_, std::move(username)),
            TransitionKind::Morph,
            true);
}

void Router::showMain(std::u32string username, std::size_t product)
{
    const bool fromProduct = dynamic_cast<ProductScreen*>(current_.get()) != nullptr;
    present(std::make_unique<MainScreen>(context_, services_, std::move(username), product),
            fromProduct ? TransitionKind::SlideBackward : TransitionKind::Morph,
            true);
}

void Router::showProduct(std::u32string username, std::size_t product)
{
    present(std::make_unique<ProductScreen>(
                context_, services_, std::move(username), product, platform::localDateText()),
            TransitionKind::SlideForward,
            true);
}

bool Router::handleUnhandledKey(const core::input::KeyEvent& event)
{
    return current_ && current_->onUnhandledKey(event);
}

}
