#include "gpu/composition/backdrop_effects.hxx"

#include <d2d1effects.h>

#include <windows.graphics.effects.interop.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Effects.h>

#include <string>
#include <vector>

namespace loader::gpu::composition
{
namespace
{

namespace wf = winrt::Windows::Foundation;
namespace wge = winrt::Windows::Graphics::Effects;
namespace wuc = winrt::Windows::UI::Composition;

constexpr UINT kBlurPropertyIndex = 0;

class D2dEffect : public winrt::implements<D2dEffect,
                                           wge::IGraphicsEffect,
                                           wge::IGraphicsEffectSource,
                                           ABI::Windows::Graphics::Effects::IGraphicsEffectD2D1Interop>
{
public:
    D2dEffect(const GUID& effectId,
              std::wstring name,
              std::wstring namedProperty = {},
              UINT namedPropertyIndex = 0)
        : effectId_(effectId), name_(std::move(name)), namedProperty_(std::move(namedProperty)),
          namedPropertyIndex_(namedPropertyIndex)
    {
    }

    void addProperty(const wf::IInspectable& value) { properties_.push_back(value.as<wf::IPropertyValue>()); }
    void addSource(const wge::IGraphicsEffectSource& source) { sources_.push_back(source); }

    winrt::hstring Name() const { return winrt::hstring(name_); }
    void Name(const winrt::hstring& name) { name_ = name.c_str(); }

    HRESULT __stdcall GetEffectId(GUID* id) noexcept override
    {
        *id = effectId_;
        return S_OK;
    }

    HRESULT __stdcall GetNamedPropertyMapping(
        LPCWSTR name,
        UINT* index,
        ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING* mapping) noexcept override
    {
        if (namedProperty_.empty() || namedProperty_ != name)
            return E_INVALIDARG;
        *index = namedPropertyIndex_;
        *mapping = ABI::Windows::Graphics::Effects::GRAPHICS_EFFECT_PROPERTY_MAPPING_DIRECT;
        return S_OK;
    }

    HRESULT __stdcall GetPropertyCount(UINT* count) noexcept override
    {
        *count = static_cast<UINT>(properties_.size());
        return S_OK;
    }

    HRESULT __stdcall GetProperty(UINT index,
                                  ABI::Windows::Foundation::IPropertyValue** value) noexcept override
    {
        if (index >= properties_.size())
            return E_INVALIDARG;
        winrt::copy_to_abi(properties_[index], *reinterpret_cast<void**>(value));
        return S_OK;
    }

    HRESULT __stdcall
    GetSource(UINT index, ABI::Windows::Graphics::Effects::IGraphicsEffectSource** source) noexcept override
    {
        if (index >= sources_.size())
            return E_INVALIDARG;
        winrt::copy_to_abi(sources_[index], *reinterpret_cast<void**>(source));
        return S_OK;
    }

    HRESULT __stdcall GetSourceCount(UINT* count) noexcept override
    {
        *count = static_cast<UINT>(sources_.size());
        return S_OK;
    }

private:
    GUID effectId_;
    std::wstring name_;
    std::wstring namedProperty_;
    UINT namedPropertyIndex_;
    std::vector<wf::IPropertyValue> properties_;
    std::vector<wge::IGraphicsEffectSource> sources_;
};

}

wuc::CompositionEffectBrush
createFrostedBackdropBrush(const wuc::Compositor& compositor, float blurStandardDeviation, float saturation)
{
    const auto backdropSource = wuc::CompositionEffectSourceParameter(kBackdropSourceName);

    auto blur =
        winrt::make_self<D2dEffect>(CLSID_D2D1GaussianBlur, L"Blur", L"BlurAmount", kBlurPropertyIndex);
    blur->addProperty(wf::PropertyValue::CreateSingle(blurStandardDeviation));
    blur->addProperty(wf::PropertyValue::CreateUInt32(D2D1_GAUSSIANBLUR_OPTIMIZATION_SPEED));
    blur->addProperty(wf::PropertyValue::CreateUInt32(D2D1_BORDER_MODE_HARD));
    blur->addSource(backdropSource);

    auto saturate = winrt::make_self<D2dEffect>(CLSID_D2D1Saturation, L"Saturate");
    saturate->addProperty(wf::PropertyValue::CreateSingle(saturation));
    saturate->addSource(blur.as<wge::IGraphicsEffectSource>());

    const wuc::CompositionEffectFactory factory = compositor.CreateEffectFactory(
        saturate.as<wge::IGraphicsEffect>(), std::vector<winrt::hstring>{kBlurAmountProperty});
    return factory.CreateBrush();
}

}
