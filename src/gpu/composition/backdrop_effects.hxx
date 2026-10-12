#pragma once

#include "platform/winrt_base.hxx"

#include <winrt/Windows.UI.Composition.h>

namespace loader::gpu::composition
{

inline constexpr wchar_t kBackdropSourceName[] = L"backdrop";
inline constexpr wchar_t kBlurAmountProperty[] = L"Blur.BlurAmount";

winrt::Windows::UI::Composition::CompositionEffectBrush
createFrostedBackdropBrush(const winrt::Windows::UI::Composition::Compositor& compositor,
                           float blurStandardDeviation,
                           float saturation);

}
