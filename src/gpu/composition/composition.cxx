#include "gpu/composition/composition.hxx"

#include "core/build_config.hxx"
#include "gpu/composition/direct_composition.hxx"
#include "gpu/composition/winrt_composition.hxx"

namespace loader::gpu
{
namespace
{

bool directCompositionForced()
{
    return core::kDebugBuild && GetEnvironmentVariableW(L"LOADER_FORCE_DCOMP", nullptr, 0) != 0;
}

}

std::unique_ptr<Composition> createComposition(HWND stageWindow)
{
    if (!directCompositionForced())
    {
        if (auto composition = tryCreateWinRtComposition(stageWindow))
            return composition;
    }
    return createDirectComposition(stageWindow);
}

}
