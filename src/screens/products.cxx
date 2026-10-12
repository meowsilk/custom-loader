#include "screens/products.hxx"

#include <array>

namespace loader::screens
{
namespace
{

constexpr std::array<ProductInfo, 10> kProducts = {{
    {U"Counter-Strike 2",
     render::ArtImage::CounterStrike2,
     ui::Str::DescCounterStrike2,
     InjectionType::Internal},
    {U"Escape from Tarkov",
     render::ArtImage::EscapeFromTarkov,
     ui::Str::DescEscapeFromTarkov,
     InjectionType::External},
    {U"Fortnite", render::ArtImage::Fortnite, ui::Str::DescFortnite, InjectionType::External},
    {U"Apex Legends", render::ArtImage::ApexLegends, ui::Str::DescApexLegends, InjectionType::Internal},
    {U"DayZ", render::ArtImage::DayZ, ui::Str::DescDayZ, InjectionType::External},
    {U"Rust", render::ArtImage::Rust, ui::Str::DescRust, InjectionType::External},
    {U"PUBG: Battlegrounds", render::ArtImage::Pubg, ui::Str::DescPubg, InjectionType::External},
    {U"Rainbow Six Siege",
     render::ArtImage::RainbowSixSiege,
     ui::Str::DescRainbowSix,
     InjectionType::Internal},
    {U"Call of Duty", render::ArtImage::CallOfDuty, ui::Str::DescCallOfDuty, InjectionType::External},
    {U"Grand Theft Auto V", render::ArtImage::GrandTheftAutoV, ui::Str::DescGtaV, InjectionType::Internal},
}};

}

std::span<const ProductInfo> products()
{
    return kProducts;
}

}
