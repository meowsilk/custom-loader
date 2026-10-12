#pragma once

#include <algorithm>
#include <cmath>

namespace loader::ui::animation
{

struct CubicBezier
{
    float x1;
    float y1;
    float x2;
    float y2;

    float operator()(float t) const
    {
        t = std::clamp(t, 0.0F, 1.0F);
        if (t == 0.0F || t == 1.0F)
            return t;

        float u = t;
        for (int iteration = 0; iteration < 8; ++iteration)
        {
            const float error = sampleX(u) - t;
            if (std::fabs(error) < 1e-5F)
                return sampleY(u);
            const float slope = derivativeX(u);
            if (std::fabs(slope) < 1e-6F)
                break;
            u -= error / slope;
        }

        float low = 0.0F;
        float high = 1.0F;
        u = t;
        for (int iteration = 0; iteration < 24; ++iteration)
        {
            const float x = sampleX(u);
            if (std::fabs(x - t) < 1e-5F)
                break;
            (x < t ? low : high) = u;
            u = (low + high) * 0.5F;
        }
        return sampleY(u);
    }

private:
    float sampleX(float u) const { return bezier(x1, x2, u); }
    float sampleY(float u) const { return bezier(y1, y2, u); }

    float derivativeX(float u) const
    {
        const float inverse = 1.0F - u;
        return 3.0F * inverse * inverse * x1 + 6.0F * inverse * u * (x2 - x1) + 3.0F * u * u * (1.0F - x2);
    }

    static float bezier(float p1, float p2, float u)
    {
        const float inverse = 1.0F - u;
        return 3.0F * inverse * inverse * u * p1 + 3.0F * inverse * u * u * p2 + u * u * u;
    }
};

namespace easing
{

inline constexpr CubicBezier standard{0.2F, 0.0F, 0.0F, 1.0F};
inline constexpr CubicBezier enter{0.0F, 0.0F, 0.2F, 1.0F};
inline constexpr CubicBezier exit{0.4F, 0.0F, 1.0F, 1.0F};
inline constexpr CubicBezier linear{0.0F, 0.0F, 1.0F, 1.0F};

}

}
