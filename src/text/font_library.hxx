#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>

#include <compare>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <span>

namespace loader::text
{

enum class FontFamily
{
    Inter,
    Icons,
};

struct FontRequest
{
    FontFamily family = FontFamily::Inter;
    float weight = 400.0F;
    float size = 14.0F;
};

struct FontVerticalMetrics
{
    float ascent = 0.0F;
    float descent = 0.0F;
};

class FontInstance
{
public:
    FontInstance(FT_Library library,
                 std::span<const std::byte> data,
                 std::optional<float> weight,
                 std::optional<float> opticalSize);
    ~FontInstance();

    FontInstance(const FontInstance&) = delete;
    FontInstance& operator=(const FontInstance&) = delete;

    FT_Face face() const { return face_; }
    hb_font_t* shaper() const { return font_; }
    float unitsPerEm() const { return static_cast<float>(face_->units_per_EM); }
    FontVerticalMetrics metrics(float size) const;
    std::uint32_t glyphForCodepoint(char32_t codepoint) const;

private:
    FT_Face face_ = nullptr;
    hb_blob_t* blob_ = nullptr;
    hb_face_t* hbFace_ = nullptr;
    hb_font_t* font_ = nullptr;
};

class FontLibrary
{
public:
    FontLibrary();
    ~FontLibrary();

    FontLibrary(const FontLibrary&) = delete;
    FontLibrary& operator=(const FontLibrary&) = delete;

    FontInstance& instance(const FontRequest& request);

private:
    struct Key
    {
        FontFamily family;
        int weight;
        int opticalSize;

        friend auto operator<=>(const Key&, const Key&) = default;
    };

    std::span<const std::byte> bytesFor(FontFamily family) const;

    FT_Library library_ = nullptr;
    std::map<Key, std::unique_ptr<FontInstance>> instances_;
};

}
