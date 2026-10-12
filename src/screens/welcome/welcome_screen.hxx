#pragma once

#include "screens/screen.hxx"
#include "ui/widgets/loading_bar.hxx"

#include <optional>
#include <string>

namespace loader::screens
{

class WelcomeScreen final : public Screen
{
public:
    WelcomeScreen(ui::UiContext& context, const Services& services, std::u32string username);
    ~WelcomeScreen() override;

    core::Size panelSize() const override { return {520.0F, 440.0F}; }

    ui::LoadingBar& bar() { return *bar_; }

private:
    void buildLayout(ui::UiContext& context);
    void scheduleHandOff();

    const Services& services_;
    std::u32string username_;
    ui::LoadingBar* bar_ = nullptr;
    std::optional<auth::TimerId> handOff_;
};

}
