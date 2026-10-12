#include "render/renderer.hxx"

#include "render/ui_pass.hxx"

namespace loader::render
{

Renderer::Renderer(gpu::Device& device, text::GlyphCache& glyphCache) : device_(device)
{
    passes_.push_back(std::make_unique<UiPass>(device_, glyphCache));
}

void Renderer::render(gpu::SwapChain& swapChain, core::SizeI stageSize, const DrawList& drawList)
{
    ID3D11DeviceContext* context = device_.context();

    const float transparent[4] = {0.0F, 0.0F, 0.0F, 0.0F};
    context->ClearRenderTargetView(swapChain.renderTarget(), transparent);

    const FrameContext frame{context, swapChain.renderTarget(), stageSize, &drawList};
    for (const auto& pass : passes_)
        pass->execute(frame);
}

}
