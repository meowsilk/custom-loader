#include "text/shaper.hxx"

#include <hb-ot.h>

#include <memory>

namespace loader::text
{
namespace
{

struct BufferDeleter
{
    void operator()(hb_buffer_t* buffer) const { hb_buffer_destroy(buffer); }
};

using UniqueBuffer = std::unique_ptr<hb_buffer_t, BufferDeleter>;

}

ShapedText shapeText(FontLibrary& fonts, const TextStyle& style, std::u32string_view text)
{
    FontInstance& font = fonts.instance({style.family, style.weight, style.size});
    const float perUnit = style.size / font.unitsPerEm();
    const FontVerticalMetrics metrics = font.metrics(style.size);

    ShapedText result;
    result.font = &font;
    result.size = style.size;
    result.ascent = metrics.ascent;
    result.descent = metrics.descent;
    result.lineHeight = style.lineHeight;

    if (text.empty())
        return result;

    const UniqueBuffer buffer(hb_buffer_create());
    hb_buffer_add_utf32(buffer.get(),
                        reinterpret_cast<const std::uint32_t*>(text.data()),
                        static_cast<int>(text.size()),
                        0,
                        static_cast<int>(text.size()));
    hb_buffer_guess_segment_properties(buffer.get());
    hb_shape(font.shaper(), buffer.get(), nullptr, 0);

    unsigned count = 0;
    const hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer.get(), &count);
    const hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer.get(), &count);

    result.glyphs.reserve(count);
    float penX = 0.0F;
    for (unsigned i = 0; i < count; ++i)
    {
        const float advance = static_cast<float>(positions[i].x_advance) * perUnit + style.tracking;
        result.glyphs.push_back({infos[i].codepoint,
                                 penX + static_cast<float>(positions[i].x_offset) * perUnit,
                                 -static_cast<float>(positions[i].y_offset) * perUnit,
                                 advance,
                                 infos[i].cluster});
        penX += advance;
    }
    result.width = penX;
    return result;
}

}
