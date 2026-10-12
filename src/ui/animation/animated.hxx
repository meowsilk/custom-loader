#pragma once

#include "ui/animation/easing.hxx"

namespace loader::ui::animation
{

class Animated
{
public:
    Animated() = default;
    explicit Animated(float initial) : value_(initial), from_(initial), target_(initial) {}

    float value() const { return value_; }
    float target() const { return target_; }
    bool animating() const { return elapsed_ < duration_; }

    void set(float value)
    {
        value_ = from_ = target_ = value;
        elapsed_ = duration_ = 0.0F;
    }

    void animateTo(float target, float duration, CubicBezier curve = easing::enter)
    {
        if (target == target_)
            return;
        if (duration <= 0.0F)
        {
            set(target);
            return;
        }
        from_ = value_;
        target_ = target;
        elapsed_ = 0.0F;
        duration_ = duration;
        curve_ = curve;
    }

    bool update(float deltaSeconds)
    {
        if (!animating())
            return false;

        elapsed_ += deltaSeconds;
        if (elapsed_ >= duration_)
        {
            value_ = target_;
            elapsed_ = duration_;
            return false;
        }
        value_ = from_ + (target_ - from_) * curve_(elapsed_ / duration_);
        return true;
    }

private:
    float value_ = 0.0F;
    float from_ = 0.0F;
    float target_ = 0.0F;
    float elapsed_ = 0.0F;
    float duration_ = 0.0F;
    CubicBezier curve_ = easing::enter;
};

}
