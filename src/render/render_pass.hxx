#pragma once

#include "core/geometry.hxx"
#include "render/draw_list.hxx"

#include <d3d11_1.h>

namespace loader::render
{

struct FrameContext
{
    ID3D11DeviceContext* context = nullptr;
    ID3D11RenderTargetView* target = nullptr;
    core::SizeI stageSize;
    const DrawList* drawList = nullptr;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;

    virtual void execute(const FrameContext& frame) = 0;
};

}
