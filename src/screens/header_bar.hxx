#pragma once

#include "screens/screen.hxx"
#include "ui/localization.hxx"
#include "ui/widgets/icon_button.hxx"

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace loader::screens
{

struct HeaderOptions
{
    std::function<void()> onBack;
    std::optional<ui::Str> titleId;
    std::u32string title;
};

std::unique_ptr<ui::Widget> makeHeaderBar(ui::UiContext& context,
                                          const Services& services,
                                          const std::u32string& username,
                                          const HeaderOptions& options);

}
