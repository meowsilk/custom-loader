#include "text/font_library.hxx"

#include FT_MULTIPLE_MASTERS_H

#include "platform/embedded_resource.hxx"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace loader::text
{
namespace
{

constexpr hb_tag_t kWeightTag = HB_TAG('w', 'g', 'h', 't');
constexpr hb_tag_t kOpticalSizeTag = HB_TAG('o', 'p', 's', 'z');
constexpr float kMinOpticalSize = 14.0F;
constexpr float kMaxOpticalSize = 32.0F;

void throwIfFreeTypeFailed(FT_Error error, const char* operation)
{
    if (error != 0)
        throw std::runtime_error(std::string(operation) + " failed");
}

FT_Fixed toFixed(float value)
{
    return static_cast<FT_Fixed>(std::lround(value * 65536.0F));
}

void applyVariations(FT_Library library,
                     FT_Face face,
                     std::optional<float> weight,
                     std::optional<float> opticalSize)
{
    FT_MM_Var* variations = nullptr;
    if (FT_Get_MM_Var(face, &variations) != 0 || variations == nullptr)
        return;

    std::vector<FT_Fixed> coordinates(variations->num_axis);
    for (FT_UInt i = 0; i < variations->num_axis; ++i)
    {
        const FT_Var_Axis& axis = variations->axis[i];
        float value = static_cast<float>(axis.def) / 65536.0F;
        if (axis.tag == kWeightTag && weight)
            value = *weight;
        else if (axis.tag == kOpticalSizeTag && opticalSize)
            value = *opticalSize;
        const float minimum = static_cast<float>(axis.minimum) / 65536.0F;
        const float maximum = static_cast<float>(axis.maximum) / 65536.0F;
        coordinates[i] = toFixed(std::clamp(value, minimum, maximum));
    }

    FT_Set_Var_Design_Coordinates(face, variations->num_axis, coordinates.data());
    FT_Done_MM_Var(library, variations);
}

}

FontInstance::FontInstance(FT_Library library,
                           std::span<const std::byte> data,
                           std::optional<float> weight,
                           std::optional<float> opticalSize)
{
    throwIfFreeTypeFailed(FT_New_Memory_Face(library,
                                             reinterpret_cast<const FT_Byte*>(data.data()),
                                             static_cast<FT_Long>(data.size()),
                                             0,
                                             &face_),
                          "FT_New_Memory_Face");
    applyVariations(library, face_, weight, opticalSize);

    blob_ = hb_blob_create(reinterpret_cast<const char*>(data.data()),
                           static_cast<unsigned>(data.size()),
                           HB_MEMORY_MODE_READONLY,
                           nullptr,
                           nullptr);
    hbFace_ = hb_face_create(blob_, 0);
    font_ = hb_font_create(hbFace_);

    std::array<hb_variation_t, 2> variationSettings{};
    unsigned count = 0;
    if (weight)
        variationSettings[count++] = {kWeightTag, *weight};
    if (opticalSize)
        variationSettings[count++] = {kOpticalSizeTag, *opticalSize};
    if (count > 0)
        hb_font_set_variations(font_, variationSettings.data(), count);
}

FontInstance::~FontInstance()
{
    hb_font_destroy(font_);
    hb_face_destroy(hbFace_);
    hb_blob_destroy(blob_);
    FT_Done_Face(face_);
}

FontVerticalMetrics FontInstance::metrics(float size) const
{
    const float perUnit = size / unitsPerEm();
    return {static_cast<float>(face_->ascender) * perUnit, -static_cast<float>(face_->descender) * perUnit};
}

std::uint32_t FontInstance::glyphForCodepoint(char32_t codepoint) const
{
    return FT_Get_Char_Index(face_, static_cast<FT_ULong>(codepoint));
}

FontLibrary::FontLibrary()
{
    throwIfFreeTypeFailed(FT_Init_FreeType(&library_), "FT_Init_FreeType");
}

FontLibrary::~FontLibrary()
{
    instances_.clear();
    FT_Done_FreeType(library_);
}

std::span<const std::byte> FontLibrary::bytesFor(FontFamily family) const
{
    switch (family)
    {
    case FontFamily::Inter:
        return platform::embeddedResource(L"INTER_VARIABLE");
    case FontFamily::Icons:
        return platform::embeddedResource(L"REMIX_LINE");
    }
    throw std::invalid_argument("unknown font family");
}

FontInstance& FontLibrary::instance(const FontRequest& request)
{
    const bool variable = request.family != FontFamily::Icons;
    const bool hasOpticalAxis = request.family == FontFamily::Inter;

    const int weight = variable ? static_cast<int>(std::lround(request.weight)) : 0;
    const int opticalSize =
        hasOpticalAxis
            ? static_cast<int>(std::lround(std::clamp(request.size, kMinOpticalSize, kMaxOpticalSize)))
            : 0;
    const Key key{request.family, weight, opticalSize};

    if (const auto found = instances_.find(key); found != instances_.end())
        return *found->second;

    auto created = std::make_unique<FontInstance>(
        library_,
        bytesFor(request.family),
        variable ? std::optional<float>(static_cast<float>(weight)) : std::nullopt,
        hasOpticalAxis ? std::optional<float>(static_cast<float>(opticalSize)) : std::nullopt);
    return *instances_.emplace(key, std::move(created)).first->second;
}

}
