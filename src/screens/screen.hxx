#pragma once

#include "auth/auth_service.hxx"
#include "auth/scheduler.hxx"
#include "core/geometry.hxx"
#include "core/input.hxx"
#include "screens/license_store.hxx"
#include "screens/products.hxx"
#include "storage/credential_vault.hxx"
#include "ui/ui_context.hxx"
#include "ui/widget.hxx"

#include <memory>
#include <optional>
#include <string>

namespace loader::screens
{

class WindowControls;

struct LoginPrefill
{
    std::u32string username;
    bool accountCreated = false;
};

class Navigator
{
public:
    virtual ~Navigator() = default;

    virtual void showLogin(std::optional<LoginPrefill> prefill) = 0;
    virtual void showRegister() = 0;
    virtual void showWelcome(std::u32string username) = 0;
    virtual void showMain(std::u32string username, std::size_t product = kDefaultProduct) = 0;
    virtual void showProduct(std::u32string username, std::size_t product) = 0;
};

struct Services
{
    auth::AuthService& auth;
    storage::CredentialVault& vault;
    auth::Scheduler& scheduler;
    Navigator& navigator;
    WindowControls& controls;
    LicenseStore& licenses;
};

class Screen
{
public:
    virtual ~Screen() = default;

    virtual core::Size panelSize() const = 0;
    virtual std::unique_ptr<ui::Widget> takeHeaderContent() { return nullptr; }
    virtual void onEnter() {}
    virtual bool onUnhandledKey(const core::input::KeyEvent&) { return false; }
    virtual ui::Widget* initialFocus() { return nullptr; }

    std::unique_ptr<ui::Widget> takeContent() { return std::move(content_); }
    ui::Widget& content() { return *content_; }

protected:
    std::unique_ptr<ui::Widget> content_;
};

}
