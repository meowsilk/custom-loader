#pragma once

#include "gpu/device.hxx"
#include "gpu/swap_chain.hxx"
#include "render/draw_list.hxx"
#include "render/render_pass.hxx"
#include "text/glyph_cache.hxx"

#include <memory>
#include <vector>

namespace loader::render
{

class Renderer
{
public:
    Renderer(gpu::Device& device, text::GlyphCache& glyphCache);

    void render(gpu::SwapChain& swapChain, core::SizeI stageSize, const DrawList& drawList);

private:
    gpu::Device& device_;
    std::vector<std::unique_ptr<RenderPass>> passes_;
};

}
