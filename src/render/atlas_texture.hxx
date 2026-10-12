#pragma once

#include "gpu/com_ptr.hxx"
#include "gpu/device.hxx"
#include "text/glyph_cache.hxx"

#include <d3d11_1.h>

namespace loader::render
{

class AtlasTexture
{
public:
    AtlasTexture(gpu::Device& device, text::GlyphCache& cache);

    void sync(ID3D11DeviceContext* context);
    ID3D11ShaderResourceView* view() const { return view_.Get(); }

private:
    text::GlyphCache& cache_;
    gpu::ComPtr<ID3D11Texture2D> texture_;
    gpu::ComPtr<ID3D11ShaderResourceView> view_;
};

}
