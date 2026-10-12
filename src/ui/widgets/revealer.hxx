#pragma once

#include "ui/animation/animated.hxx"
#include "ui/widget.hxx"

namespace loader::ui
{

class Revealer : public Widget
{
public:
    explicit Revealer(UiContext& context);

    bool revealed() const { return revealed_; }
    void setRevealed(bool revealed, bool animate = true);

protected:
    bool updateSelf(float deltaSeconds) override;

private:
    void apply();

    animation::Animated amount_;
    bool revealed_ = false;
};

}
