#pragma once

#include <array>
#include <cstddef>

namespace loader::render
{

enum class ArtImage : int
{
    CounterStrike2 = 0,
    EscapeFromTarkov = 1,
    Fortnite = 2,
    ApexLegends = 3,
    DayZ = 4,
    Rust = 5,
    Pubg = 6,
    RainbowSixSiege = 7,
    CallOfDuty = 8,
    GrandTheftAutoV = 9,
};

inline constexpr std::size_t kArtImageCount = 10;

inline constexpr std::array<const wchar_t*, kArtImageCount> kArtImageResources = {
    L"IMAGE_COUNTER_STRIKE_2",
    L"IMAGE_ESCAPE_FROM_TARKOV",
    L"IMAGE_FORTNITE",
    L"IMAGE_APEX_LEGENDS",
    L"IMAGE_DAYZ",
    L"IMAGE_RUST",
    L"IMAGE_PUBG",
    L"IMAGE_RAINBOW_SIX_SIEGE",
    L"IMAGE_CALL_OF_DUTY",
    L"IMAGE_GRAND_THEFT_AUTO_V",
};

enum class FlagImage : int
{
    English = 0,
    German = 1,
    Spanish = 2,
    Russian = 3,
    French = 4,
    Portuguese = 5,
    Turkish = 6,
    Polish = 7,
    Italian = 8,
};

inline constexpr std::size_t kFlagImageCount = 9;

inline constexpr std::array<const wchar_t*, kFlagImageCount> kFlagImageResources = {
    L"FLAG_ENGLISH",
    L"FLAG_GERMAN",
    L"FLAG_SPANISH",
    L"FLAG_RUSSIAN",
    L"FLAG_FRENCH",
    L"FLAG_PORTUGUESE",
    L"FLAG_TURKISH",
    L"FLAG_POLISH",
    L"FLAG_ITALIAN",
};

}
