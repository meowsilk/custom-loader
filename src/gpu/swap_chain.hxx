#pragma once

#include "core/geometry.hxx"
#include "gpu/com_ptr.hxx"
#include "gpu/device.hxx"
#include "platform/unique_handle.hxx"

#include <d3d11_1.h>
#include <dxgi1_3.h>

namespace loader::gpu
{

class SwapChain
{
public:
    SwapChain(Device& device, core::SizeI size);

    void resize(core::SizeI size);
    bool present();

    IDXGISwapChain1* handle() const { return swapChain_.Get(); }
    ID3D11RenderTargetView* renderTarget() const { return renderTarget_.Get(); }
    ID3D11Texture2D* backBuffer() const { return backBuffer_.Get(); }
    HANDLE frameReadyEvent() const { return frameLatencyWaitable_.get(); }

private:
    void acquireBackBuffer();

    Device& device_;
    core::SizeI size_;
    ComPtr<IDXGISwapChain2> swapChain_;
    ComPtr<ID3D11Texture2D> backBuffer_;
    ComPtr<ID3D11RenderTargetView> renderTarget_;
    platform::UniqueHandle frameLatencyWaitable_;
};

}
