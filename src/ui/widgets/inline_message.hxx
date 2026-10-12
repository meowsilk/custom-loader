#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"
#include "ui/widgets/text_tone.hxx"

#include <optional>
#include <string>

namespace loader::ui
{

class InlineMessage : public Widget
{
public:
    explicit InlineMessage(UiContext& context);

    void show(const std::u32string& message, TextTone tone);
    void show(Str id, TextTone tone);
    void clear();
    bool showing() const { return reveal_.target() > 0.5F; }

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;

private:
    void present(std::u32string_view message, TextTone tone);

    text::ShapedText message_;
    text::ShapedText icon_;
    TextTone tone_ = TextTone::Danger;
    animation::Animated reveal_;
    std::optional<Str> id_;
    Localization::Subscription languageSubscription_;
};

}
