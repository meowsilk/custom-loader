#pragma once

#include "app/system_clipboard.hxx"
#include "app/ui_scheduler.hxx"
#include "auth/mock_auth_service.hxx"
#include "gpu/composition/composition.hxx"
#include "gpu/device.hxx"
#include "gpu/swap_chain.hxx"
#include "platform/message_loop.hxx"
#include "platform/overlay_window.hxx"
#include "platform/window_events.hxx"
#include "render/draw_list.hxx"
#include "render/renderer.hxx"
#include "screens/router.hxx"
#include "screens/shell.hxx"
#include "screens/warmup.hxx"
#include "storage/app_paths.hxx"
#include "storage/credential_vault.hxx"
#include "storage/log_file.hxx"
#include "storage/settings.hxx"
#include "text/font_library.hxx"
#include "text/glyph_cache.hxx"
#include "ui/backdrop_physics.hxx"
#include "ui/input/input_router.hxx"
#include "ui/theme/theme.hxx"
#include "ui/ui_context.hxx"

#include <chrono>
#include <memory>
#include <utility>
#include <vector>

namespace loader::app
{

class Application final : public platform::WindowEvents,
                          public platform::FrameClient,
                          public screens::WindowControls
{
public:
    Application();
    ~Application() override;

    int run();

    bool isCaption(core::Vec2 stagePoint) const override;
    core::input::Cursor cursorAt(core::Vec2 stagePoint) const override;
    void onStageChanged() override;
    void onCloseRequested() override;
    void onActivationChanged(bool active) override;
    void onSystemSettingsChanged() override;
    void onPointerMoved(core::Vec2 stagePoint) override;
    void onPointerLeft() override;
    void onPointerButton(core::input::PointerButton button, bool down, core::Vec2 stagePoint) override;
    void onWheel(core::Vec2 stagePoint, float notches) override;
    void onKeyDown(const core::input::KeyEvent& event) override;
    void onChar(char32_t codePoint) override;

    bool wantsFrame() const override { return frameRequested_; }
    HANDLE frameReadyEvent() const override;
    void renderFrame() override;
    std::optional<std::chrono::milliseconds> idleTimeout() const override;
    void onIdleTimeout() override { requestFrame(); }

    void minimizeWindow() override;
    void closeWindow() override;

private:
    using Clock = std::chrono::steady_clock;

    struct GpuResources
    {
        GpuResources(core::SizeI swapChainSize, text::GlyphCache& glyphCache);

        gpu::Device device;
        gpu::SwapChain swapChain;
        render::Renderer renderer;
    };

    void buildGpuResources();
    void syncStage();
    void syncPanelShape();
    void syncImeState();
    void refreshSystemState();
    void requestFrame() { frameRequested_ = true; }
    core::Vec2 toLogical(core::Vec2 stagePoint) const;
    void warmGlyphs(ui::UiContext& context, screens::WarmScreen screen);
    void scheduleLanguageWarmup();
    void warmNextLanguage();
    void rebuildGpuAfterLoss();

    text::FontLibrary fonts_;
    text::GlyphCache glyphCache_;
    ui::theme::Theme theme_;
    platform::OverlayWindow window_;
    SystemClipboard clipboard_;
    UiScheduler scheduler_;
    storage::AppPaths paths_;
    storage::Logger logger_;
    storage::CredentialVault vault_;
    storage::Settings settings_;
    auth::MockAuthService auth_;
    ui::UiContext ui_;
    ui::Localization::Subscription languageSubscription_;
    ui::input::InputRouter input_;
    ui::BackdropPhysics backdrop_;
    std::unique_ptr<screens::Shell> shell_;
    std::unique_ptr<screens::Router> router_;
    std::unique_ptr<gpu::Composition> composition_;
    std::unique_ptr<GpuResources> gpu_;
    render::DrawList drawList_;
    std::vector<std::pair<ui::Language, screens::WarmScreen>> languageWarmQueue_;
    core::Rect appliedPanel_;
    Clock::time_point lastFrame_ = Clock::now();
    bool frameRequested_ = true;
    std::size_t debugDpiIndex_ = 0;
};

}
