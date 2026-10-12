#include "gpu/composition/direct_composition.hxx"

#include "gpu/com_ptr.hxx"
#include "platform/win32_error.hxx"

#include <dcomp.h>

namespace loader::gpu
{
namespace
{

class DirectComposition final : public Composition
{
public:
    explicit DirectComposition(HWND stageWindow)
    {
        platform::throwIfFailed(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&device_)),
                                "DCompositionCreateDevice");
        platform::throwIfFailed(device_->CreateTargetForHwnd(stageWindow, TRUE, &target_),
                                "IDCompositionDevice::CreateTargetForHwnd");
        platform::throwIfFailed(device_->CreateVisual(&visual_), "IDCompositionDevice::CreateVisual");
        platform::throwIfFailed(target_->SetRoot(visual_.Get()), "IDCompositionTarget::SetRoot");
    }

    void attach(IDXGISwapChain1& swapChain, core::SizeI stagePixels) override
    {
        platform::throwIfFailed(visual_->SetContent(&swapChain), "IDCompositionVisual::SetContent");
        resize(stagePixels);
    }

    void resize(core::SizeI stagePixels) override
    {
        const D2D_RECT_F clip{
            0.0F, 0.0F, static_cast<float>(stagePixels.width), static_cast<float>(stagePixels.height)};
        platform::throwIfFailed(visual_->SetClip(clip), "IDCompositionVisual::SetClip");
        platform::throwIfFailed(device_->Commit(), "IDCompositionDevice::Commit");
    }

    void setPanel(core::Rect) override {}
    void setBackdropBlur(float) override {}
    void refreshBackdropAvailability() override {}
    bool hasBackdrop() const override { return false; }
    std::string_view name() const override { return "DirectComposition"; }

private:
    ComPtr<IDCompositionDevice> device_;
    ComPtr<IDCompositionTarget> target_;
    ComPtr<IDCompositionVisual> visual_;
};

}

std::unique_ptr<Composition> createDirectComposition(HWND stageWindow)
{
    return std::make_unique<DirectComposition>(stageWindow);
}

}
