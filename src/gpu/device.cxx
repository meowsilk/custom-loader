#include "gpu/device.hxx"

#include "core/build_config.hxx"
#include "platform/win32_error.hxx"

#include <dxgi1_6.h>

#include <array>

namespace loader::gpu
{
namespace
{

ComPtr<IDXGIAdapter1> pickDisplayAdapter()
{
    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        return nullptr;

    ComPtr<IDXGIFactory6> factory6;
    if (FAILED(factory.As(&factory6)))
        return nullptr;

    ComPtr<IDXGIAdapter1> adapter;
    if (FAILED(factory6->EnumAdapterByGpuPreference(
            0, DXGI_GPU_PREFERENCE_MINIMUM_POWER, IID_PPV_ARGS(&adapter))))
        return nullptr;

    DXGI_ADAPTER_DESC1 description{};
    if (FAILED(adapter->GetDesc1(&description)) || (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
        return nullptr;
    return adapter;
}

constexpr std::array kFeatureLevels = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

HRESULT createDevice(IDXGIAdapter* adapter,
                     D3D_DRIVER_TYPE driver,
                     UINT flags,
                     ComPtr<ID3D11Device>& device,
                     ComPtr<ID3D11DeviceContext>& context)
{
    return D3D11CreateDevice(adapter,
                             adapter != nullptr ? D3D_DRIVER_TYPE_UNKNOWN : driver,
                             nullptr,
                             flags,
                             kFeatureLevels.data(),
                             static_cast<UINT>(kFeatureLevels.size()),
                             D3D11_SDK_VERSION,
                             device.ReleaseAndGetAddressOf(),
                             nullptr,
                             context.ReleaseAndGetAddressOf());
}

HRESULT
createDeviceWithFallbacks(UINT flags, ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context)
{
    const ComPtr<IDXGIAdapter1> displayAdapter = pickDisplayAdapter();

    struct Attempt
    {
        IDXGIAdapter* adapter;
        D3D_DRIVER_TYPE driver;
    };
    const std::array attempts = {Attempt{displayAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN},
                                 Attempt{nullptr, D3D_DRIVER_TYPE_HARDWARE},
                                 Attempt{nullptr, D3D_DRIVER_TYPE_WARP}};

    for (const Attempt& attempt : attempts)
    {
        if (attempt.adapter == nullptr && attempt.driver == D3D_DRIVER_TYPE_UNKNOWN)
            continue;

        HRESULT result = createDevice(attempt.adapter, attempt.driver, flags, device, context);
        if (FAILED(result) && (flags & D3D11_CREATE_DEVICE_DEBUG) != 0)
        {
            result = createDevice(attempt.adapter,
                                  attempt.driver,
                                  flags & ~static_cast<UINT>(D3D11_CREATE_DEVICE_DEBUG),
                                  device,
                                  context);
        }
        if (SUCCEEDED(result))
            return result;
    }
    return E_FAIL;
}

}

Device::Device()
{
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    if constexpr (core::kDebugBuild)
        flags |= D3D11_CREATE_DEVICE_DEBUG;

    platform::throwIfFailed(createDeviceWithFallbacks(flags, device_, context_), "D3D11CreateDevice");
    platform::throwIfFailed(device_.As(&dxgiDevice_), "QueryInterface(IDXGIDevice)");

    ComPtr<IDXGIAdapter> adapter;
    platform::throwIfFailed(dxgiDevice_->GetAdapter(&adapter), "IDXGIDevice::GetAdapter");
    platform::throwIfFailed(adapter->GetParent(IID_PPV_ARGS(&factory_)),
                            "IDXGIAdapter::GetParent(IDXGIFactory2)");
}

}
