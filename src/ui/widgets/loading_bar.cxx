#include "ui/widgets/loading_bar.hxx"

#include "ui/animation/easing.hxx"

#include <algorithm>
#include <cmath>
#include <string>

namespace loader::ui
{
namespace
{

constexpr float kWidth = 300.0F;
constexpr float kTrackHeight = 6.0F;
constexpr float kLabelGap = 10.0F;
constexpr float kStartDelay = 0.5F;
constexpr float kDuration = 2.4F;
constexpr float kHold = 0.35F;
constexpr float kShimmerSpeed = 230.0F;
constexpr float kShimmerWidth = 64.0F;
constexpr animation::CubicBezier kProgressCurve{0.45F, 0.05F, 0.2F, 1.0F};

constexpr std::array<float, 3> kStageStarts = {0.0F, 0.34F, 0.72F};
constexpr std::array<Str, 3> kStageIds = {Str::StagePreparing, Str::StageSyncing, Str::StageAlmostReady};

}

LoadingBar::LoadingBar(UiContext& context) : Widget(context)
{
    style().width = kWidth;
    reshapeStages();
    languageSubscription_ = context_.localization().subscribe(
        [this]
        {
            reshapeStages();
            invalidate();
        });
    applyProgress(0.0F);
}

void LoadingBar::reshapeStages()
{
    const theme::TypeScale& type = context_.theme().type;
    for (std::size_t i = 0; i < kStageCount; ++i)
        stages_[i] =
            text::shapeText(context_.fonts(), type.label, context_.localization().text(kStageIds[i]));
}

core::Size LoadingBar::measureContent()
{
    return {kWidth, kTrackHeight + kLabelGap + context_.theme().type.label.lineHeight};
}

void LoadingBar::applyProgress(float progress)
{
    progress_ = std::clamp(progress, 0.0F, 1.0F);

    std::size_t stage = 0;
    for (std::size_t i = 0; i < kStageCount; ++i)
    {
        if (progress_ >= kStageStarts[i])
            stage = i;
    }
    if (stage != stage_)
    {
        stage_ = stage;
        stageFade_.set(0.0F);
        stageFade_.animateTo(1.0F, context_.theme().motion.slow);
    }

    const int percent = static_cast<int>(std::floor(progress_ * 100.0F + 0.5F));
    if (percent != shownPercent_)
    {
        shownPercent_ = percent;
        std::u32string label;
        for (const char digit : std::to_string(percent))
            label.push_back(static_cast<char32_t>(digit));
        label.push_back(U'%');
        percent_ = text::shapeText(context_.fonts(), context_.theme().type.label, label);
    }
}

void LoadingBar::seek(float progress)
{
    applyProgress(progress);
    invalidate();
}

bool LoadingBar::updateSelf(float deltaSeconds)
{
    shimmer_ += deltaSeconds;
    bool animating = stageFade_.update(deltaSeconds);

    if (!notified_)
    {
        elapsed_ += deltaSeconds;
        const float local = std::clamp((elapsed_ - kStartDelay) / kDuration, 0.0F, 1.0F);
        applyProgress(kProgressCurve(local));

        if (local >= 1.0F)
        {
            hold_ += deltaSeconds;
            if (hold_ >= kHold)
            {
                notified_ = true;
                if (onComplete_)
                    onComplete_();
            }
        }
        animating = true;
    }
    return animating || progress_ > 0.0F;
}

void LoadingBar::paintSelf(PaintContext& context)
{
    const theme::Theme& theme = context.theme();
    const core::Rect box = bounds();
    const render::Corners corners = render::Corners::uniform(kTrackHeight * 0.5F);

    context.useFront();
    context.draw().addRect({{box.x, box.y, box.width, kTrackHeight},
                            corners,
                            core::Color::fromHex(0xFFFFFF, 0.14F),
                            core::Color::fromHex(0xFFFFFF, 0.10F),
                            1.0F});

    const float fillWidth = box.width * progress_;
    if (fillWidth > 1.0F)
    {
        const core::Rect fill{box.x, box.y, std::max(fillWidth, kTrackHeight), kTrackHeight};
        const float glow = std::min(1.0F, progress_ * 6.0F);

        context.draw().addShadow(
            {fill, kTrackHeight * 0.5F, {0.0F, 0.0F}, 8.0F, core::Color::fromHex(0xFFFFFF, 0.42F * glow)});
        context.draw().addRect({fill, corners, core::Color::fromHex(0xF4F5F7, 0.94F)});

        const float travel = box.width + kShimmerWidth * 2.0F;
        const float bandX = box.x - kShimmerWidth + std::fmod(shimmer_ * kShimmerSpeed, travel);
        context.draw().pushClip(fill, kTrackHeight * 0.5F);
        const float widths[3] = {kShimmerWidth, kShimmerWidth * 0.6F, kShimmerWidth * 0.28F};
        const float alphas[3] = {0.16F, 0.26F, 0.5F};
        for (int i = 0; i < 3; ++i)
        {
            context.draw().addRect({{bandX - widths[i] * 0.5F, box.y, widths[i], kTrackHeight},
                                    render::Corners{},
                                    core::Color::fromHex(0xFFFFFF, alphas[i])});
        }
        context.draw().popClip();
    }

    const float labelY = box.y + kTrackHeight + kLabelGap;
    const float lineHeight = theme.type.label.lineHeight;

    text::ShapedText status = stages_[stage_];
    status.lineHeight = lineHeight;
    context.draw().addTextWithShadow(
        status, {box.x, labelY}, theme.colors.textSecondary.scaledAlpha(stageFade_.value()));

    text::ShapedText percent = percent_;
    percent.lineHeight = lineHeight;
    context.draw().addTextWithShadow(
        percent, {box.right() - percent_.width, labelY}, theme.colors.textPrimary);
}

}
