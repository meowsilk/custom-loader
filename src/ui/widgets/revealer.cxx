#include "ui/widgets/revealer.hxx"

namespace loader::ui
{
namespace
{

constexpr float kRise = 8.0F;

}

Revealer::Revealer(UiContext& context) : Widget(context)
{
    setVisible(false);
}

void Revealer::setRevealed(bool revealed, bool animate)
{
    if (revealed_ == revealed)
        return;
    revealed_ = revealed;
    setVisible(revealed);

    if (revealed && animate)
    {
        amount_.set(0.0F);
        amount_.animateTo(1.0F, context_.theme().motion.slow);
    }
    else
    {
        amount_.set(revealed ? 1.0F : 0.0F);
    }
    apply();
}

void Revealer::apply()
{
    const float amount = amount_.value();
    setPresentation({0.0F, kRise * (1.0F - amount)}, amount);
}

bool Revealer::updateSelf(float deltaSeconds)
{
    const bool animating = amount_.update(deltaSeconds);
    apply();
    return animating;
}

}
