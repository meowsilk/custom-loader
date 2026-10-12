#include "gpu/composition/winrt_composition.hxx"

#include "gpu/composition/backdrop_effects.hxx"
#include "platform/win32_error.hxx"

#include <DispatcherQueue.h>

#include <windows.ui.composition.interop.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.ViewManagement.h>

#include <cmath>
#include <exception>

namespace loader::gpu
{
namespace
{

namespace wuc = winrt::Windows::UI::Composition;

constexpr float kBackdropSaturation = 1.35F;
constexpr float kInitialBlur = 1.0F;

using CreateDispatcherQueueControllerFn =
    HRESULT(WINAPI*)(DispatcherQueueOptions, ABI::Windows::System::IDispatcherQueueController**);

winrt::Windows::System::DispatcherQueueController createDispatcherQueueController()
{
    const HMODULE coreMessaging = LoadLibraryExW(L"CoreMessaging.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (coreMessaging == nullptr)
        platform::throwLastError("LoadLibrary(CoreMessaging)");

    const auto create = reinterpret_cast<CreateDispatcherQueueControllerFn>(
        GetProcAddress(coreMessaging, "CreateDispatcherQueueController"));
    if (create == nullptr)
        platform::throwLastError("GetProcAddress(CreateDispatcherQueueController)");

    const DispatcherQueueOptions options{
        sizeof(DispatcherQueueOptions), DQTYPE_THREAD_CURRENT, DQTAT_COM_NONE};
    winrt::Windows::System::DispatcherQueueController controller{nullptr};
    winrt::check_hresult(create(
        options,
        reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(winrt::put_abi(controller))));
    return controller;
}

wuc::CompositionSurfaceBrush createPixelBrush(const wuc::Compositor& compositor,
                                              const wuc::ICompositionSurface& surface)
{
    auto brush = compositor.CreateSurfaceBrush(surface);
    brush.Stretch(wuc::CompositionStretch::None);
    brush.HorizontalAlignmentRatio(0.0F);
    brush.VerticalAlignmentRatio(0.0F);
    brush.BitmapInterpolationMode(wuc::CompositionBitmapInterpolationMode::NearestNeighbor);
    return brush;
}

class WinRtComposition final : public Composition
{
public:
    explicit WinRtComposition(HWND stageWindow)
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
        dispatcherQueue_ = createDispatcherQueueController();

        compositor_ = wuc::Compositor();

        const auto desktopInterop =
            compositor_.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();
        winrt::check_hresult(desktopInterop->CreateDesktopWindowTarget(
            stageWindow,
            FALSE,
            reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**>(
                winrt::put_abi(target_))));

        root_ = compositor_.CreateContainerVisual();
        target_.Root(root_);

        backdropBrush_ =
            composition::createFrostedBackdropBrush(compositor_, kInitialBlur, kBackdropSaturation);
        backdropBrush_.SetSourceParameter(composition::kBackdropSourceName,
                                          compositor_.CreateHostBackdropBrush());

        backdropVisual_ = compositor_.CreateSpriteVisual();
        contentVisual_ = compositor_.CreateSpriteVisual();
        root_.Children().InsertAtBottom(backdropVisual_);
        root_.Children().InsertAtTop(contentVisual_);

        uiSettings_ = winrt::Windows::UI::ViewManagement::UISettings();
        refreshBackdropAvailability();
    }

    void attach(IDXGISwapChain1& swapChain, core::SizeI stagePixels) override
    {
        const auto compositorInterop = compositor_.as<ABI::Windows::UI::Composition::ICompositorInterop>();
        winrt::com_ptr<ABI::Windows::UI::Composition::ICompositionSurface> abiSurface;
        winrt::check_hresult(
            compositorInterop->CreateCompositionSurfaceForSwapChain(&swapChain, abiSurface.put()));

        wuc::ICompositionSurface surface{nullptr};
        winrt::copy_from_abi(surface, abiSurface.get());

        contentVisual_.Brush(createPixelBrush(compositor_, surface));

        maskSurfaceBrush_ = createPixelBrush(compositor_, surface);
        auto maskBrush = compositor_.CreateMaskBrush();
        maskBrush.Source(backdropBrush_);
        maskBrush.Mask(maskSurfaceBrush_);
        backdropVisual_.Brush(maskBrush);

        resize(stagePixels);
    }

    void resize(core::SizeI stagePixels) override
    {
        stagePixels_ = stagePixels;
        contentVisual_.Size({static_cast<float>(stagePixels.width), static_cast<float>(stagePixels.height)});
        applyPanel();
    }

    void setPanel(core::Rect panelPixels) override
    {
        panel_ = panelPixels;
        applyPanel();
    }

    void setBackdropBlur(float standardDeviationPixels) override
    {
        backdropBrush_.Properties().InsertScalar(composition::kBlurAmountProperty, standardDeviationPixels);
    }

    void refreshBackdropAvailability() override
    {
        bool available = true;
        try
        {
            available = uiSettings_.AdvancedEffectsEnabled();
        }
        catch (const winrt::hresult_error&)
        {
            available = true;
        }
        backdropAvailable_ = available;
        backdropVisual_.IsVisible(available);
    }

    bool hasBackdrop() const override { return backdropAvailable_; }
    std::string_view name() const override { return "Windows.UI.Composition"; }

private:
    void applyPanel()
    {
        if (maskSurfaceBrush_ == nullptr)
            return;

        const float left = std::floor(panel_.x);
        const float top = std::floor(panel_.y);
        const float right = std::ceil(panel_.right());
        const float bottom = std::ceil(panel_.bottom());

        backdropVisual_.Offset({left, top, 0.0F});
        backdropVisual_.Size({right - left, bottom - top});
        maskSurfaceBrush_.Offset({-(static_cast<float>(stagePixels_.width) + left), -top});
    }

    winrt::Windows::System::DispatcherQueueController dispatcherQueue_{nullptr};
    wuc::Compositor compositor_{nullptr};
    wuc::Desktop::DesktopWindowTarget target_{nullptr};
    wuc::ContainerVisual root_{nullptr};
    wuc::SpriteVisual backdropVisual_{nullptr};
    wuc::SpriteVisual contentVisual_{nullptr};
    wuc::CompositionEffectBrush backdropBrush_{nullptr};
    wuc::CompositionSurfaceBrush maskSurfaceBrush_{nullptr};
    winrt::Windows::UI::ViewManagement::UISettings uiSettings_{nullptr};
    bool backdropAvailable_ = true;
    core::SizeI stagePixels_;
    core::Rect panel_;
};

}

std::unique_ptr<Composition> tryCreateWinRtComposition(HWND stageWindow)
{
    try
    {
        return std::make_unique<WinRtComposition>(stageWindow);
    }
    catch (const winrt::hresult_error&)
    {
        return nullptr;
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
}

}
