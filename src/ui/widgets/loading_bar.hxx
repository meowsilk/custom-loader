#pragma once

#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/widget.hxx"

#include <array>
#include <functional>

namespace loader::ui
{

class LoadingBar : public Widget
{
public:
    explicit LoadingBar(UiContext& context);

    void setOnComplete(std::function<void()> handler) { onComplete_ = std::move(handler); }
    void seek(float progress);

    float progress() const { return progress_; }
    bool finished() const { return notified_; }
    std::size_t stage() const { return stage_; }

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;

private:
    static constexpr std::size_t kStageCount = 3;

    void applyProgress(float progress);
    void reshapeStages();

    std::array<text::ShapedText, kStageCount> stages_;
    text::ShapedText percent_;
    int shownPercent_ = -1;
    std::size_t stage_ = 0;
    float progress_ = 0.0F;
    float elapsed_ = 0.0F;
    float hold_ = 0.0F;
    float shimmer_ = 0.0F;
    bool notified_ = false;
    animation::Animated stageFade_{1.0F};
    std::function<void()> onComplete_;
    Localization::Subscription languageSubscription_;
};

}
