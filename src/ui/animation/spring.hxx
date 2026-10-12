#pragma once

#include <algorithm>
#include <cmath>

namespace loader::ui::animation
{

class Spring
{
public:
    Spring() = default;
    explicit Spring(float initial) : value_(initial), target_(initial) {}

    float value() const { return value_; }
    float target() const { return target_; }
    bool animating() const { return !settled_; }

    void set(float value)
    {
        value_ = target_ = value;
        velocity_ = 0.0F;
        settled_ = true;
    }

    void animateTo(float target, float dampingRatio = 0.85F, float angularFrequency = 14.0F)
    {
        if (target == target_ && settled_)
            return;
        target_ = target;
        dampingRatio_ = dampingRatio;
        angularFrequency_ = angularFrequency;
        settled_ = false;
    }

    bool update(float deltaSeconds)
    {
        if (settled_)
            return false;

        constexpr float kMaxStep = 1.0F / 240.0F;
        float remaining = std::min(deltaSeconds, 0.1F);
        while (remaining > 0.0F)
        {
            const float step = std::min(remaining, kMaxStep);
            const float displacement = value_ - target_;
            const float acceleration = -angularFrequency_ * angularFrequency_ * displacement -
                                       2.0F * dampingRatio_ * angularFrequency_ * velocity_;
            velocity_ += acceleration * step;
            value_ += velocity_ * step;
            remaining -= step;
        }

        if (std::fabs(value_ - target_) < 0.01F && std::fabs(velocity_) < 0.05F)
        {
            value_ = target_;
            velocity_ = 0.0F;
            settled_ = true;
            return false;
        }
        return true;
    }

private:
    float value_ = 0.0F;
    float target_ = 0.0F;
    float velocity_ = 0.0F;
    float dampingRatio_ = 0.85F;
    float angularFrequency_ = 14.0F;
    bool settled_ = true;
};

}
