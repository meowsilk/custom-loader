#pragma once

#include "gpu/com_ptr.hxx"
#include "gpu/device.hxx"
#include "render/atlas_texture.hxx"
#include "render/draw_list.hxx"
#include "render/render_pass.hxx"
#include "text/glyph_cache.hxx"

#include <d3d11_1.h>

#include <array>
#include <span>

namespace loader::render
{

class UiPass final : public RenderPass
{
public:
    UiPass(gpu::Device& device, text::GlyphCache& glyphCache);

    void execute(const FrameContext& frame) override;

private:
    struct LayerRange
    {
        UINT first = 0;
        UINT count = 0;
    };

    struct UploadResult
    {
        std::array<LayerRange, kLayerCount> layers;
        LayerRange mask;
    };

    void ensureInstanceCapacity(std::size_t count);
    void ensureSceneTexture(core::SizeI size);
    gpu::ComPtr<ID3D11ShaderResourceView> createTextureArray(std::span<const wchar_t* const> resources);
    UploadResult uploadInstances(const FrameContext& frame);
    void drawRange(ID3D11DeviceContext* context,
                   const D3D11_VIEWPORT& viewport,
                   const LayerRange& range,
                   core::SizeI stage);
    void drawLayer(const FrameContext& frame, const LayerRange& range, ID3D11BlendState* blend);
    void copyScene(const FrameContext& frame);
    void uploadBackdropField(const FrameContext& frame);

    gpu::Device& device_;
    gpu::ComPtr<ID3D11VertexShader> vertexShader_;
    gpu::ComPtr<ID3D11PixelShader> pixelShader_;
    gpu::ComPtr<ID3D11Buffer> constants_;
    gpu::ComPtr<ID3D11Buffer> backdropConstants_;
    gpu::ComPtr<ID3D11Buffer> instances_;
    gpu::ComPtr<ID3D11ShaderResourceView> instancesView_;
    gpu::ComPtr<ID3D11BlendState> overBlend_;
    gpu::ComPtr<ID3D11BlendState> replaceBlend_;
    gpu::ComPtr<ID3D11RasterizerState> rasterizerState_;
    gpu::ComPtr<ID3D11SamplerState> glyphSampler_;
    gpu::ComPtr<ID3D11Texture2D> sceneTexture_;
    gpu::ComPtr<ID3D11ShaderResourceView> sceneView_;
    gpu::ComPtr<ID3D11ShaderResourceView> artView_;
    gpu::ComPtr<ID3D11ShaderResourceView> flagView_;
    core::SizeI sceneSize_;
    AtlasTexture atlas_;
    std::size_t instanceCapacity_ = 0;
};

}
