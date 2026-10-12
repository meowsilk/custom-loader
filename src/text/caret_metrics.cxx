#include "text/caret_metrics.hxx"

#include <algorithm>
#include <cmath>

namespace loader::text
{

std::vector<float> caretOffsets(const ShapedText& shaped, std::size_t textLength)
{
    std::vector<float> offsets(textLength + 1, 0.0F);

    float pen = 0.0F;
    std::size_t index = 0;
    std::size_t covered = 0;
    while (index < shaped.glyphs.size())
    {
        const std::uint32_t cluster = shaped.glyphs[index].cluster;
        float runAdvance = 0.0F;
        std::size_t end = index;
        while (end < shaped.glyphs.size() && shaped.glyphs[end].cluster == cluster)
            runAdvance += shaped.glyphs[end++].advance;

        const std::size_t nextCluster = end < shaped.glyphs.size() ? shaped.glyphs[end].cluster : textLength;
        const std::size_t span = std::max<std::size_t>(1, nextCluster - cluster);
        for (std::size_t k = 0; k < span && cluster + k <= textLength; ++k)
            offsets[cluster + k] = pen + runAdvance * static_cast<float>(k) / static_cast<float>(span);

        covered = std::max<std::size_t>(covered, cluster + span);
        pen += runAdvance;
        index = end;
    }

    for (std::size_t i = covered; i <= textLength; ++i)
        offsets[i] = pen;
    return offsets;
}

std::size_t caretIndexAt(const std::vector<float>& offsets, float x)
{
    std::size_t best = 0;
    float bestDistance = std::fabs(x - offsets.front());
    for (std::size_t i = 1; i < offsets.size(); ++i)
    {
        const float distance = std::fabs(x - offsets[i]);
        if (distance < bestDistance)
        {
            best = i;
            bestDistance = distance;
        }
    }
    return best;
}

}
