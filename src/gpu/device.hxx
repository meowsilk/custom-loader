#pragma once

#include "gpu/com_ptr.hxx"

#include <d3d11_1.h>
#include <dxgi1_3.h>

namespace loader::gpu
{

class Device
{
public:
    Device();

    ID3D11Device* device() const { return device_.Get(); }
    ID3D11DeviceContext* context() const { return context_.Get(); }
    IDXGIFactory2* factory() const { return factory_.Get(); }

private:
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGIDevice> dxgiDevice_;
    ComPtr<IDXGIFactory2> factory_;
};

}
