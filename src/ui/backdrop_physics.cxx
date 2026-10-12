#include "ui/backdrop_physics.hxx"

#include <algorithm>
#include <cmath>

namespace loader::ui
{
namespace
{

constexpr float kFieldRadius = 96.0F;
constexpr float kMaxStep = 1.0F / 240.0F;
constexpr float kMaxSpeed = 3200.0F;
constexpr float kRippleLifetime = 1.8F;
constexpr float kEnterRate = 9.0F;
constexpr float kLeaveRate = 4.5F;
constexpr float kSettleDistance = 0.05F;
constexpr float kSettleSpeed = 0.5F;
constexpr float kSettlePresence = 0.002F;

struct SpringTuning
{
    float angularFrequency;
    float dampingRatio;
};

constexpr SpringTuning kLively{11.0F, 0.38F};
constexpr SpringTuning kCalm{15.0F, 1.0F};

float length(core::Vec2 vector)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

}

void BackdropPhysics::pointerMoved(core::Vec2 position)
{
    if (!inside_)
    {
        inside_ = true;
        if (presence_ < kSettlePresence)
        {
            position_ = position;
            velocity_ = {};
        }
    }
    target_ = position;
}

void BackdropPhysics::pointerLeft()
{
    inside_ = false;
}

void BackdropPhysics::pointerPressed(core::Vec2 position)
{
    if (calm_)
        return;

    RippleState* slot = &ripples_[0];
    for (RippleState& ripple : ripples_)
    {
        if (!ripple.active)
        {
            slot = &ripple;
            break;
        }
        if (ripple.age > slot->age)
            slot = &ripple;
    }
    *slot = {position, 0.0F, true};
}

bool BackdropPhysics::settled() const
{
    if (std::fabs(presence_ - (inside_ ? 1.0F : 0.0F)) > kSettlePresence)
        return false;
    if (length(position_ - target_) > kSettleDistance || length(velocity_) > kSettleSpeed)
        return false;
    return std::ranges::none_of(ripples_, [](const RippleState& r) { return r.active; });
}

bool BackdropPhysics::update(float deltaSeconds)
{
    if (settled())
        return false;

    const SpringTuning tuning = calm_ ? kCalm : kLively;
    float remaining = std::min(deltaSeconds, 0.1F);
    while (remaining > 0.0F)
    {
        const float step = std::min(remaining, kMaxStep);
        const core::Vec2 displacement = position_ - target_;
        const core::Vec2 acceleration = displacement * (-tuning.angularFrequency * tuning.angularFrequency) -
                                        velocity_ * (2.0F * tuning.dampingRatio * tuning.angularFrequency);
        velocity_ = velocity_ + acceleration * step;
        position_ = position_ + velocity_ * step;
        remaining -= step;
    }

    const float speed = length(velocity_);
    if (speed > kMaxSpeed)
        velocity_ = velocity_ * (kMaxSpeed / speed);

    const float goal = inside_ ? 1.0F : 0.0F;
    const float rate = inside_ ? kEnterRate : kLeaveRate;
    presence_ += (goal - presence_) * (1.0F - std::exp(-rate * deltaSeconds));
    if (std::fabs(presence_ - goal) <= kSettlePresence)
        presence_ = goal;

    for (RippleState& ripple : ripples_)
    {
        if (!ripple.active)
            continue;
        ripple.age += deltaSeconds;
        if (ripple.age >= kRippleLifetime)
            ripple.active = false;
    }

    if (settled())
    {
        position_ = target_;
        velocity_ = {};
        return false;
    }
    return true;
}

render::BackdropField BackdropPhysics::field() const
{
    render::BackdropField field;
    field.position = target_;
    field.trail = position_;
    field.velocity = velocity_;
    field.presence = presence_;
    field.radius = kFieldRadius;
    for (std::size_t i = 0; i < ripples_.size(); ++i)
    {
        const RippleState& ripple = ripples_[i];
        if (ripple.active)
            field.ripples[i] = {ripple.origin, ripple.age, 1.0F - ripple.age / kRippleLifetime};
    }
    return field;
}

}
