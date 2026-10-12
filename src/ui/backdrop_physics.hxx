#pragma once

#include "core/geometry.hxx"
#include "render/backdrop_field.hxx"

#include <array>

namespace loader::ui
{

class BackdropPhysics
{
public:
    void setCalm(bool calm) { calm_ = calm; }

    void pointerMoved(core::Vec2 position);
    void pointerLeft();
    void pointerPressed(core::Vec2 position);

    bool update(float deltaSeconds);

    render::BackdropField field() const;

private:
    struct RippleState
    {
        core::Vec2 origin;
        float age = 0.0F;
        bool active = false;
    };

    bool settled() const;

    core::Vec2 target_;
    core::Vec2 position_;
    core::Vec2 velocity_;
    float presence_ = 0.0F;
    bool inside_ = false;
    bool calm_ = false;
    std::array<RippleState, render::kMaxRipples> ripples_{};
};

}
