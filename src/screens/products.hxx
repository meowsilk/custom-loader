#pragma once

#include "render/art_images.hxx"
#include "ui/localization.hxx"

#include <cstddef>
#include <span>

namespace loader::screens
{

enum class InjectionType
{
    Internal,
    External,
};

struct ProductInfo
{
    const char32_t* name;
    render::ArtImage art;
    ui::Str description;
    InjectionType injection;
};

inline constexpr std::size_t kDefaultProduct = 2;

std::span<const ProductInfo> products();

}
