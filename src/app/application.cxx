#include "app/application.hxx"

#include "core/build_config.hxx"
#include "ui/widgets/text_field.hxx"

#include <algorithm>
#include <array>

namespace loader::app
{
namespace
{

constexpr core::Size kStageSize{1088.0F, 748.0F};
constexpr core::Size kLoginPanelSize{420.0F, 540.0F};
constexpr float kBackdropBlurSigma = 14.0F;
constexpr float kScreenMargin = 32.0F;
constexpr std::chrono::milliseconds kLanguageWarmDelay{1500};
constexpr std::chrono::milliseconds kLanguageWarmStep{120};
constexpr std::chrono::milliseconds kLanguageWarmRetry{250};
constexpr std::array kStartupWarmScreens = {screens::WarmScreen::Register,
                                            screens::WarmScreen::Welcome,
                                            screens::WarmScreen::Main,
                                            screens::WarmScreen::Product};
constexpr std::array<unsigned, 4> kDebugDpis = {96, 120, 144, 192};

core::SizeI swapChainSizeFor(core::SizeI stage)
{
    return {stage.width * 2, stage.height};
}

bool systemAnimationsEnabled()
{
    if (core::kDebugBuild && GetEnvironmentVariableW(L"LOADER_FORCE_MOTION", nullptr, 0) != 0)
        return true;

    BOOL enabled = TRUE;
    SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &enabled, 0);
    return enabled != FALSE;
}

}

Application::GpuResources::GpuResources(core::SizeI swapChainSize, text::GlyphCache& glyphCache)
    : swapChain(device, swapChainSize), renderer(device, glyphCache)
{
}

Application::Application()
    : theme_(ui::theme::makeDarkGlassTheme()),
      window_(*this, platform::OverlayDesc{kStageSize, kLoginPanelSize, theme_.radii.panel, L"loader.gg"}),
      clipboard_(window_.inputHandle()), logger_(paths_), vault_(paths_), settings_(paths_),
      auth_(scheduler_), ui_(theme_, fonts_, clipboard_, [this] { requestFrame(); })
{
    drawList_.setGlyphCache(glyphCache_);
    input_.observe(ui_);

    if (const std::optional<std::string> code = settings_.language())
    {
        if (const std::optional<ui::Language> language = ui::languageFromCode(*code))
            ui_.localization().setLanguage(*language);
    }
    languageSubscription_ = ui_.localization().subscribe(
        [this] { settings_.setLanguage(ui::languageCode(ui_.localization().language())); });

    shell_ = std::make_unique<screens::Shell>(ui_, *this);
    router_ = std::make_unique<screens::Router>(ui_, *shell_, input_, auth_, vault_, scheduler_, *this);
    input_.setUnhandledKeyHandler([this](const core::input::KeyEvent& event)
                                  { return router_->handleUnhandledKey(event); });

    composition_ = gpu::createComposition(window_.stageHandle());
    buildGpuResources();
    refreshSystemState();
    syncStage();

    router_->start();
    for (const screens::WarmScreen screen : kStartupWarmScreens)
        warmGlyphs(ui_, screen);
    scheduleLanguageWarmup();
    logger_.info("application started");
}

Application::~Application() = default;

void Application::buildGpuResources()
{
    const core::SizeI stage = window_.stagePixels();
    gpu_ = std::make_unique<GpuResources>(swapChainSizeFor(stage), glyphCache_);
    composition_->attach(*gpu_->swapChain.handle(), stage);
}

void Application::refreshSystemState()
{
    composition_->refreshBackdropAvailability();
    shell_->setBackdropAvailable(composition_->hasBackdrop());
    const bool animationsEnabled = systemAnimationsEnabled();
    shell_->setReducedMotion(!animationsEnabled);
    backdrop_.setCalm(!animationsEnabled);
}

void Application::syncStage()
{
    const core::Size workArea = window_.workAreaLogical();
    shell_->setMaxPanelSize({workArea.width - kScreenMargin, workArea.height - kScreenMargin});

    const core::SizeI stage = window_.stagePixels();
    gpu_->swapChain.resize(swapChainSizeFor(stage));
    composition_->resize(stage);
    composition_->setBackdropBlur(kBackdropBlurSigma * window_.scale());
    appliedPanel_ = {};
    requestFrame();
}

void Application::syncPanelShape()
{
    const core::Rect panel = shell_->panelRect();
    if (panel == appliedPanel_)
        return;

    appliedPanel_ = panel;
    window_.setPanel(panel, theme_.radii.panel);
    composition_->setPanel(window_.panelRectPixels());
}

core::Vec2 Application::toLogical(core::Vec2 stagePoint) const
{
    const float scale = window_.scale();
    return {stagePoint.x / scale, stagePoint.y / scale};
}

int Application::run()
{
    renderFrame();
    window_.show();
    return platform::runMessageLoop(*this);
}

bool Application::isCaption(core::Vec2 stagePoint) const
{
    if (router_->inputBlocked())
        return false;

    const core::Vec2 logical = toLogical(stagePoint);
    return shell_->headerRect().contains(logical) && shell_->hitTest(logical) == nullptr;
}

core::input::Cursor Application::cursorAt(core::Vec2 stagePoint) const
{
    if (router_->inputBlocked())
        return core::input::Cursor::Arrow;

    const ui::Widget* hit = shell_->hitTest(toLogical(stagePoint));
    return hit != nullptr && hit->enabled() ? hit->cursorKind() : core::input::Cursor::Arrow;
}

void Application::onStageChanged()
{
    syncStage();
}

void Application::onCloseRequested()
{
    closeWindow();
}

void Application::onActivationChanged(bool active)
{
    input_.windowActivationChanged(active);
    requestFrame();
}

void Application::onSystemSettingsChanged()
{
    refreshSystemState();
    requestFrame();
}

void Application::onPointerMoved(core::Vec2 stagePoint)
{
    backdrop_.pointerMoved(toLogical(stagePoint));
    requestFrame();
    if (router_->inputBlocked())
        return;
    input_.pointerMoved(toLogical(stagePoint));
}

void Application::onPointerLeft()
{
    backdrop_.pointerLeft();
    requestFrame();
    input_.pointerLeft();
}

void Application::onPointerButton(core::input::PointerButton button, bool down, core::Vec2 stagePoint)
{
    if (down && button == core::input::PointerButton::Left)
    {
        backdrop_.pointerPressed(toLogical(stagePoint));
        requestFrame();
    }
    if (router_->inputBlocked())
        return;
    input_.pointerButton(button, down, toLogical(stagePoint));
}

void Application::onWheel(core::Vec2 stagePoint, float notches)
{
    if (router_->inputBlocked())
        return;
    input_.wheel(toLogical(stagePoint), notches);
}

void Application::onKeyDown(const core::input::KeyEvent& event)
{
    if constexpr (core::kDebugBuild)
    {
        if (event.key == core::input::Key::F9)
        {
            rebuildGpuAfterLoss();
            return;
        }
        if (event.key == core::input::Key::F10)
        {
            debugDpiIndex_ = (debugDpiIndex_ + 1) % kDebugDpis.size();
            const unsigned dpi = kDebugDpis[debugDpiIndex_];
            SendMessageW(window_.inputHandle(), WM_DPICHANGED, MAKEWPARAM(dpi, dpi), 0);
            return;
        }
    }

    if (router_->inputBlocked())
        return;
    input_.keyDown(event);
}

void Application::onChar(char32_t codePoint)
{
    if (router_->inputBlocked())
        return;
    input_.character(codePoint);
}

void Application::minimizeWindow()
{
    window_.minimize();
}

void Application::closeWindow()
{
    PostQuitMessage(0);
}

HANDLE Application::frameReadyEvent() const
{
    return gpu_ ? gpu_->swapChain.frameReadyEvent() : nullptr;
}

std::optional<std::chrono::milliseconds> Application::idleTimeout() const
{
    std::optional<std::chrono::milliseconds> timeout;
    const auto consider = [&](std::chrono::milliseconds candidate)
    { timeout = timeout ? std::min(*timeout, candidate) : candidate; };

    if (const std::optional<float> wake = shell_->wakeDelay())
        consider(std::chrono::milliseconds(static_cast<long long>(*wake * 1000.0F) + 1));
    if (const std::optional<std::chrono::milliseconds> next = scheduler_.timeUntilNext())
        consider(*next + std::chrono::milliseconds(1));
    return timeout;
}

void Application::syncImeState()
{
    auto* field = dynamic_cast<ui::TextField*>(input_.focus().focused());
    if (field == nullptr || !field->visible())
    {
        window_.setImeState(false, {});
        return;
    }

    const core::Rect caret = field->caretRect();
    const float scale = window_.scale();
    window_.setImeState(!field->secret(),
                        {caret.x * scale, caret.y * scale, caret.width * scale, caret.height * scale});
}

void Application::warmGlyphs(ui::UiContext& context, screens::WarmScreen screen)
{
    render::DrawList scratch;
    scratch.setGlyphCache(glyphCache_);
    scratch.begin(window_.scale(), window_.stagePixels());
    ui::PaintContext paint(scratch, context);
    screens::warmUp(context, router_->services(), paint, screen);
}

void Application::scheduleLanguageWarmup()
{
    for (const ui::Language language : ui::kLanguages)
    {
        if (language == ui_.localization().language())
            continue;
        for (const screens::WarmScreen screen : screens::kWarmScreens)
            languageWarmQueue_.emplace_back(language, screen);
    }
    std::ranges::reverse(languageWarmQueue_);
    scheduler_.post(kLanguageWarmDelay, [this] { warmNextLanguage(); });
}

void Application::warmNextLanguage()
{
    if (languageWarmQueue_.empty())
        return;

    if (frameRequested_ || router_->inputBlocked())
    {
        scheduler_.post(kLanguageWarmRetry, [this] { warmNextLanguage(); });
        return;
    }

    const auto [language, screen] = languageWarmQueue_.back();
    languageWarmQueue_.pop_back();

    ui::UiContext scratchContext(theme_, fonts_, clipboard_, [] {});
    scratchContext.localization().setLanguage(language);
    warmGlyphs(scratchContext, screen);

    if (!languageWarmQueue_.empty())
        scheduler_.post(kLanguageWarmStep, [this] { warmNextLanguage(); });
}

void Application::rebuildGpuAfterLoss()
{
    gpu_.reset();
    buildGpuResources();
    syncStage();
}

void Application::renderFrame()
{
    frameRequested_ = false;

    const Clock::time_point now = Clock::now();
    const float deltaSeconds = std::min(0.1F, std::chrono::duration<float>(now - lastFrame_).count());
    lastFrame_ = now;

    scheduler_.runDue();
    const bool backdropAnimating = backdrop_.update(deltaSeconds);
    const bool animating = shell_->update(deltaSeconds) || backdropAnimating;
    router_->update();

    const core::Size stage = window_.stageSizeLogical();
    shell_->layout({0.0F, 0.0F, stage.width, stage.height});
    syncPanelShape();

    drawList_.begin(window_.scale(), window_.stagePixels());
    ui::PaintContext paint(drawList_, ui_);
    drawList_.setBackdropField(backdrop_.field());
    shell_->paint(paint);

    gpu_->renderer.render(gpu_->swapChain, window_.stagePixels(), drawList_);
    syncImeState();

    if (animating)
        requestFrame();

    if (!gpu_->swapChain.present())
        rebuildGpuAfterLoss();
}

}
