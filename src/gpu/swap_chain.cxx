#include "gpu/swap_chain.hxx"

#include "platform/win32_error.hxx"

#include <algorithm>

namespace loader::gpu
{
namespace
{

constexpr UINT kSwapChainFlags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
constexpr std::int32_t kMaxTextureExtent = 16384;

core::SizeI clampedSize(core::SizeI size)
{
    return {std::clamp(size.width, 1, kMaxTextureExtent), std::clamp(size.height, 1, kMaxTextureExtent)};
}

}

SwapChain::SwapChain(Device& device, core::SizeI size) : device_(device), size_(clampedSize(size))
{
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = static_cast<UINT>(size_.width);
    desc.Height = static_cast<UINT>(size_.height);
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    desc.Flags = kSwapChainFlags;

    ComPtr<IDXGISwapChain1> swapChain;
    platform::throwIfFailed(
        device_.factory()->CreateSwapChainForComposition(device_.device(), &desc, nullptr, &swapChain),
        "CreateSwapChainForComposition");
    platform::throwIfFailed(swapChain.As(&swapChain_), "QueryInterface(IDXGISwapChain2)");
    platform::throwIfFailed(swapChain_->SetMaximumFrameLatency(1), "SetMaximumFrameLatency");
    frameLatencyWaitable_ = platform::UniqueHandle(swapChain_->GetFrameLatencyWaitableObject());

    acquireBackBuffer();
}

void SwapChain::acquireBackBuffer()
{
    platform::throwIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer_.ReleaseAndGetAddressOf())),
                            "IDXGISwapChain::GetBuffer");
    platform::throwIfFailed(device_.device()->CreateRenderTargetView(
                                backBuffer_.Get(), nullptr, renderTarget_.ReleaseAndGetAddressOf()),
                            "CreateRenderTargetView");
}

void SwapChain::resize(core::SizeI size)
{
    const core::SizeI target = clampedSize(size);
    if (target == size_)
        return;

    renderTarget_.Reset();
    backBuffer_.Reset();
    device_.context()->ClearState();
    device_.context()->Flush();

    platform::throwIfFailed(swapChain_->ResizeBuffers(0,
                                                      static_cast<UINT>(target.width),
                                                      static_cast<UINT>(target.height),
                                                      DXGI_FORMAT_UNKNOWN,
                                                      kSwapChainFlags),
                            "IDXGISwapChain::ResizeBuffers");
    size_ = target;
    acquireBackBuffer();
}

bool SwapChain::present()
{
    const HRESULT result = swapChain_->Present(1, 0);
    if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
        return false;
    platform::throwIfFailed(result, "IDXGISwapChain::Present");
    return true;
}

}
