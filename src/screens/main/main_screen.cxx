#include "screens/main/main_screen.hxx"

#include "screens/header_bar.hxx"
#include "screens/products.hxx"
#include "ui/widgets/stack.hxx"

namespace loader::screens
{

MainScreen::MainScreen(ui::UiContext& context,
                       const Services& services,
                       std::u32string username,
                       std::size_t startProduct)
    : services_(services), username_(std::move(username))
{
    buildHeader(context);
    buildContent(context, startProduct);
}

void MainScreen::buildHeader(ui::UiContext& context)
{
    HeaderOptions options;
    options.titleId = ui::Str::OurProducts;
    header_ = makeHeaderBar(context, services_, username_, options);
}

void MainScreen::buildContent(ui::UiContext& context, std::size_t startProduct)
{
    auto root = std::make_unique<ui::Stack>(
        context, ui::layout::Axis::Column, 0.0F, core::Insets{16.0F, 0.0F, 16.0F, 16.0F});

    carousel_ = &root->add<ui::ProductCarousel>();
    carousel_->style().flexGrow = 1.0F;
    for (const ProductInfo& product : products())
        carousel_->addProduct(product.name, product.art);
    carousel_->jumpTo(startProduct);
    carousel_->setOnActivate([this](std::size_t index)
                             { services_.navigator.showProduct(username_, index); });

    content_ = std::move(root);
}

}
