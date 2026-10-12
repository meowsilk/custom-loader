#include "render/atlas_texture.hxx"

#include "platform/win32_error.hxx"

namespace loader::render
{

AtlasTexture::AtlasTexture(gpu::Device& device, text::GlyphCache& cache) : cache_(cache)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = text::GlyphCache::kAtlasSize;
    desc.Height = text::GlyphCache::kAtlasSize;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    const D3D11_SUBRESOURCE_DATA initial{
        cache_.pixels().data(), text::GlyphCache::kAtlasSize * text::GlyphCache::kBytesPerPixel, 0};
    platform::throwIfFailed(device.device()->CreateTexture2D(&desc, &initial, &texture_),
                            "CreateTexture2D(atlas)");
    platform::throwIfFailed(device.device()->CreateShaderResourceView(texture_.Get(), nullptr, &view_),
                            "CreateShaderResourceView(atlas)");
    cache_.takeDirtyRegion();
}

void AtlasTexture::sync(ID3D11DeviceContext* context)
{
    const std::optional<text::AtlasRegion> region = cache_.takeDirtyRegion();
    if (!region)
        return;

    const D3D11_BOX box{static_cast<UINT>(region->left),
                        static_cast<UINT>(region->top),
                        0,
                        static_cast<UINT>(region->right),
                        static_cast<UINT>(region->bottom),
                        1};
    const std::size_t rowPitch =
        static_cast<std::size_t>(text::GlyphCache::kAtlasSize) * text::GlyphCache::kBytesPerPixel;
    const std::uint8_t* source = cache_.pixels().data() + static_cast<std::size_t>(region->top) * rowPitch +
                                 static_cast<std::size_t>(region->left) * text::GlyphCache::kBytesPerPixel;
    context->UpdateSubresource(texture_.Get(), 0, &box, source, static_cast<UINT>(rowPitch), 0);
}

}
