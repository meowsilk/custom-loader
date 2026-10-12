#include "text/msdf_generator.hxx"

#include FT_OUTLINE_H

#include <msdfgen.h>

#include <algorithm>
#include <cmath>

namespace loader::text
{
namespace
{

struct OutlineReader
{
    msdfgen::Shape* shape = nullptr;
    msdfgen::Contour* contour = nullptr;
    msdfgen::Point2 position;
};

msdfgen::Point2 toPoint(const FT_Vector& vector)
{
    return {static_cast<double>(vector.x), static_cast<double>(vector.y)};
}

int moveTo(const FT_Vector* to, void* user)
{
    auto* reader = static_cast<OutlineReader*>(user);
    if (reader->contour == nullptr || !reader->contour->edges.empty())
        reader->contour = &reader->shape->addContour();
    reader->position = toPoint(*to);
    return 0;
}

int lineTo(const FT_Vector* to, void* user)
{
    auto* reader = static_cast<OutlineReader*>(user);
    const msdfgen::Point2 end = toPoint(*to);
    if (end != reader->position)
    {
        reader->contour->addEdge(msdfgen::EdgeHolder(reader->position, end));
        reader->position = end;
    }
    return 0;
}

int conicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
    auto* reader = static_cast<OutlineReader*>(user);
    const msdfgen::Point2 end = toPoint(*to);
    if (end != reader->position)
    {
        reader->contour->addEdge(msdfgen::EdgeHolder(reader->position, toPoint(*control), end));
        reader->position = end;
    }
    return 0;
}

int cubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user)
{
    auto* reader = static_cast<OutlineReader*>(user);
    const msdfgen::Point2 end = toPoint(*to);
    reader->contour->addEdge(
        msdfgen::EdgeHolder(reader->position, toPoint(*control1), toPoint(*control2), end));
    reader->position = end;
    return 0;
}

bool readOutline(FT_Face face, std::uint32_t glyphId, msdfgen::Shape& shape)
{
    if (FT_Load_Glyph(face, glyphId, FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP) != 0)
        return false;
    if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
        return false;

    OutlineReader reader;
    reader.shape = &shape;
    shape.setYAxisOrientation(msdfgen::Y_UPWARD);

    FT_Outline_Funcs functions{};
    functions.move_to = &moveTo;
    functions.line_to = &lineTo;
    functions.conic_to = &conicTo;
    functions.cubic_to = &cubicTo;
    if (FT_Outline_Decompose(&face->glyph->outline, &functions, &reader) != 0)
        return false;

    if (!shape.contours.empty() && shape.contours.back().edges.empty())
        shape.contours.pop_back();
    return !shape.contours.empty();
}

std::uint8_t toByte(float value)
{
    return static_cast<std::uint8_t>(std::lround(std::clamp(value, 0.0F, 1.0F) * 255.0F));
}

}

std::optional<GlyphBitmap> generateGlyphBitmap(const FontInstance& font, std::uint32_t glyphId)
{
    msdfgen::Shape shape;
    if (!readOutline(font.face(), glyphId, shape))
        return std::nullopt;

    shape.normalize();
    msdfgen::edgeColoringSimple(shape, 3.0);

    const msdfgen::Shape::Bounds bounds = shape.getBounds();
    const double unitsPerEm = font.unitsPerEm();
    const double scale = static_cast<double>(kGlyphEmPixels) / unitsPerEm;
    const double padding = std::ceil(kGlyphRangePixels * 0.5) + 1.0;

    const int width = static_cast<int>(std::ceil((bounds.r - bounds.l) * scale + 2.0 * padding));
    const int height = static_cast<int>(std::ceil((bounds.t - bounds.b) * scale + 2.0 * padding));
    if (width <= 0 || height <= 0)
        return std::nullopt;

    const msdfgen::Vector2 translate(padding / scale - bounds.l, padding / scale - bounds.b);
    const msdfgen::SDFTransformation transformation(
        msdfgen::Projection(msdfgen::Vector2(scale, scale), translate),
        msdfgen::Range(static_cast<double>(kGlyphRangePixels) / scale));

    msdfgen::MSDFGeneratorConfig generatorConfig;
    generatorConfig.overlapSupport = true;
    generatorConfig.errorCorrection.mode = msdfgen::ErrorCorrectionConfig::DISABLED;

    msdfgen::Bitmap<float, 3> field(width, height);
    msdfgen::generateMSDF(field, shape, transformation, generatorConfig);
    msdfgen::distanceSignCorrection(field, shape, transformation, 0.5F, msdfgen::FILL_NONZERO);

    msdfgen::MSDFGeneratorConfig postCorrectionConfig;
    postCorrectionConfig.errorCorrection.distanceCheckMode =
        msdfgen::ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
    msdfgen::msdfErrorCorrection(field, shape, transformation, postCorrectionConfig);

    GlyphBitmap result;
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
    for (int y = 0; y < height; ++y)
    {
        const int sourceRow = height - 1 - y;
        for (int x = 0; x < width; ++x)
        {
            const float* texel = field(x, sourceRow);
            std::uint8_t* destination =
                &result.rgba[(static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                              static_cast<std::size_t>(x)) *
                             4];
            destination[0] = toByte(texel[0]);
            destination[1] = toByte(texel[1]);
            destination[2] = toByte(texel[2]);
            destination[3] = 255;
        }
    }

    const double padUnits = padding / scale;
    result.planeLeft = static_cast<float>((bounds.l - padUnits) / unitsPerEm);
    result.planeRight = static_cast<float>((bounds.l - padUnits + width / scale) / unitsPerEm);
    result.planeBottom = static_cast<float>((bounds.b - padUnits) / unitsPerEm);
    result.planeTop = static_cast<float>((bounds.b - padUnits + height / scale) / unitsPerEm);
    return result;
}

}
