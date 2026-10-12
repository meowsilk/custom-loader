#pragma once

#include "screens/screen.hxx"
#include "ui/widgets/product_carousel.hxx"

#include <string>

namespace loader::screens
{

class MainScreen final : public Screen
{
public:
    MainScreen(ui::UiContext& context,
               const Services& services,
               std::u32string username,
               std::size_t startProduct = kDefaultProduct);

    core::Size panelSize() const override { return {980.0F, 580.0F}; }
    std::unique_ptr<ui::Widget> takeHeaderContent() override { return std::move(header_); }

    ui::ProductCarousel& carousel() { return *carousel_; }

private:
    void buildHeader(ui::UiContext& context);
    void buildContent(ui::UiContext& context, std::size_t startProduct);

    const Services& services_;
    std::u32string username_;
    std::unique_ptr<ui::Widget> header_;
    ui::ProductCarousel* carousel_ = nullptr;
};

}
