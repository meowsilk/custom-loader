#pragma once

#include "core/geometry.hxx"

#include <dxgi1_3.h>

#include <memory>
#include <string_view>

namespace loader::gpu
{

class Composition
{
public:
    virtual ~Composition() = default;

    virtual void attach(IDXGISwapChain1& swapChain, core::SizeI stagePixels) = 0;
    virtual void resize(core::SizeI stagePixels) = 0;
    virtual void setPanel(core::Rect panelPixels) = 0;
    virtual void setBackdropBlur(float standardDeviationPixels) = 0;
    virtual void refreshBackdropAvailability() = 0;
    virtual bool hasBackdrop() const = 0;
    virtual std::string_view name() const = 0;
};

std::unique_ptr<Composition> createComposition(HWND stageWindow);

}
