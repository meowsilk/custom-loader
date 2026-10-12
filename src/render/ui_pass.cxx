#include "render/ui_pass.hxx"

#include "gpu/shader_bytecode.hxx"
#include "platform/embedded_resource.hxx"
#include "platform/image_decoder.hxx"
#include "platform/win32_error.hxx"

#include <algorithm>
#include <cstring>
#include <vector>

namespace loader::render
{
namespace
{

constexpr std::size_t kInitialInstanceCapacity = 256;

struct FrameConstants
{
    float stageSize[2];
    std::uint32_t baseInstance;
    std::uint32_t padding;
};

static_assert(sizeof(FrameConstants) == 16);

struct BackdropConstants
{
    float position[2];
    float velocity[2];
    float presence;
    float radius;
    float trail[2];
    float ripples[kMaxRipples][4];
};

static_assert(sizeof(BackdropConstants) == 96);

gpu::ComPtr<ID3D11BlendState> createBlendState(ID3D11Device* device, bool premultipliedOver)
{
    D3D11_BLEND_DESC desc{};
    D3D11_RENDER_TARGET_BLEND_DESC& target = desc.RenderTarget[0];
    target.BlendEnable = premultipliedOver ? TRUE : FALSE;
    target.SrcBlend = D3D11_BLEND_ONE;
    target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    target.BlendOp = D3D11_BLEND_OP_ADD;
    target.SrcBlendAlpha = D3D11_BLEND_ONE;
    target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    gpu::ComPtr<ID3D11BlendState> state;
    platform::throwIfFailed(device->CreateBlendState(&desc, &state), "CreateBlendState(ui)");
    return state;
}

}

UiPass::UiPass(gpu::Device& device, text::GlyphCache& glyphCache)
    : device_(device), atlas_(device, glyphCache)
{
    ID3D11Device* d3d = device_.device();

    const gpu::ShaderBytecode vertex = gpu::shaders::uiVertex();
    const gpu::ShaderBytecode pixel = gpu::shaders::uiPixel();
    platform::throwIfFailed(
        d3d->CreateVertexShader(vertex.bytes.data(), vertex.bytes.size(), nullptr, &vertexShader_),
        "CreateVertexShader(ui)");
    platform::throwIfFailed(
        d3d->CreatePixelShader(pixel.bytes.data(), pixel.bytes.size(), nullptr, &pixelShader_),
        "CreatePixelShader(ui)");

    D3D11_BUFFER_DESC constantsDesc{};
    constantsDesc.ByteWidth = sizeof(FrameConstants);
    constantsDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantsDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    platform::throwIfFailed(d3d->CreateBuffer(&constantsDesc, nullptr, &constants_),
                            "CreateBuffer(ui constants)");

    D3D11_BUFFER_DESC backdropDesc = constantsDesc;
    backdropDesc.ByteWidth = sizeof(BackdropConstants);
    platform::throwIfFailed(d3d->CreateBuffer(&backdropDesc, nullptr, &backdropConstants_),
                            "CreateBuffer(backdrop constants)");

    overBlend_ = createBlendState(d3d, true);
    replaceBlend_ = createBlendState(d3d, false);

    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;
    platform::throwIfFailed(d3d->CreateRasterizerState(&rasterizerDesc, &rasterizerState_),
                            "CreateRasterizerState(ui)");

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    platform::throwIfFailed(d3d->CreateSamplerState(&samplerDesc, &glyphSampler_),
                            "CreateSamplerState(glyph)");

    ensureInstanceCapacity(kInitialInstanceCapacity);
    artView_ = createTextureArray(kArtImageResources);
    flagView_ = createTextureArray(kFlagImageResources);
}

gpu::ComPtr<ID3D11ShaderResourceView> UiPass::createTextureArray(std::span<const wchar_t* const> resources)
{
    std::vector<platform::DecodedImage> images;
    images.reserve(resources.size());
    for (const wchar_t* name : resources)
    {
        images.push_back(platform::decodeImage(platform::embeddedResource(name)));
        if (images.back().size != images.front().size)
            throw platform::SystemError("texture array size mismatch", E_INVALIDARG);
    }

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(images.front().size.width);
    desc.Height = static_cast<UINT>(images.front().size.height);
    desc.MipLevels = 0;
    desc.ArraySize = static_cast<UINT>(images.size());
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    gpu::ComPtr<ID3D11Texture2D> texture;
    platform::throwIfFailed(device_.device()->CreateTexture2D(&desc, nullptr, &texture),
                            "CreateTexture2D(texture array)");
    texture->GetDesc(&desc);

    ID3D11DeviceContext* context = device_.context();
    for (std::size_t i = 0; i < images.size(); ++i)
    {
        const UINT subresource = D3D11CalcSubresource(0, static_cast<UINT>(i), desc.MipLevels);
        context->UpdateSubresource(
            texture.Get(), subresource, nullptr, images[i].bgra.data(), desc.Width * 4U, 0);
    }

    gpu::ComPtr<ID3D11ShaderResourceView> view;
    platform::throwIfFailed(device_.device()->CreateShaderResourceView(texture.Get(), nullptr, &view),
                            "CreateShaderResourceView(texture array)");
    context->GenerateMips(view.Get());
    return view;
}

void UiPass::ensureInstanceCapacity(std::size_t count)
{
    if (count <= instanceCapacity_)
        return;

    std::size_t capacity = std::max(instanceCapacity_, kInitialInstanceCapacity);
    while (capacity < count)
        capacity *= 2;

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = static_cast<UINT>(capacity * sizeof(ShapeInstance));
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(ShapeInstance);

    gpu::ComPtr<ID3D11Buffer> buffer;
    platform::throwIfFailed(device_.device()->CreateBuffer(&desc, nullptr, &buffer),
                            "CreateBuffer(ui instances)");

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
    viewDesc.Format = DXGI_FORMAT_UNKNOWN;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    viewDesc.Buffer.NumElements = static_cast<UINT>(capacity);

    gpu::ComPtr<ID3D11ShaderResourceView> view;
    platform::throwIfFailed(device_.device()->CreateShaderResourceView(buffer.Get(), &viewDesc, &view),
                            "CreateShaderResourceView(ui instances)");

    instances_ = std::move(buffer);
    instancesView_ = std::move(view);
    instanceCapacity_ = capacity;
}

void UiPass::ensureSceneTexture(core::SizeI size)
{
    if (sceneTexture_ && sceneSize_ == size)
        return;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(size.width);
    desc.Height = static_cast<UINT>(size.height);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    platform::throwIfFailed(device_.device()->CreateTexture2D(&desc, nullptr, &sceneTexture_),
                            "CreateTexture2D(scene)");
    platform::throwIfFailed(
        device_.device()->CreateShaderResourceView(sceneTexture_.Get(), nullptr, &sceneView_),
        "CreateShaderResourceView(scene)");
    sceneSize_ = size;
}

UiPass::UploadResult UiPass::uploadInstances(const FrameContext& frame)
{
    UploadResult result;

    std::size_t total = 0;
    for (std::size_t i = 0; i < kLayerCount; ++i)
        total += frame.drawList->layer(static_cast<Layer>(i)).size();
    const std::optional<ShapeInstance> mask = frame.drawList->windowMask();
    ensureInstanceCapacity(total + 1);

    D3D11_MAPPED_SUBRESOURCE mapped{};
    platform::throwIfFailed(frame.context->Map(instances_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                            "Map(ui instances)");
    auto* destination = static_cast<ShapeInstance*>(mapped.pData);

    UINT cursor = 0;
    for (std::size_t i = 0; i < kLayerCount; ++i)
    {
        const std::span<const ShapeInstance> shapes = frame.drawList->layer(static_cast<Layer>(i));
        std::ranges::copy(shapes, destination + cursor);
        result.layers[i] = {cursor, static_cast<UINT>(shapes.size())};
        cursor += static_cast<UINT>(shapes.size());
    }
    if (mask)
    {
        destination[cursor] = *mask;
        result.mask = {cursor, 1};
    }
    frame.context->Unmap(instances_.Get(), 0);
    return result;
}

void UiPass::drawRange(ID3D11DeviceContext* context,
                       const D3D11_VIEWPORT& viewport,
                       const LayerRange& range,
                       core::SizeI stage)
{
    FrameConstants values{};
    values.stageSize[0] = static_cast<float>(stage.width);
    values.stageSize[1] = static_cast<float>(stage.height);
    values.baseInstance = range.first;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    platform::throwIfFailed(context->Map(constants_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                            "Map(ui constants)");
    std::memcpy(mapped.pData, &values, sizeof(values));
    context->Unmap(constants_.Get(), 0);

    context->RSSetViewports(1, &viewport);
    context->DrawInstanced(4, range.count, 0, 0);
}

void UiPass::drawLayer(const FrameContext& frame, const LayerRange& range, ID3D11BlendState* blend)
{
    if (range.count == 0)
        return;

    const D3D11_VIEWPORT viewport{0.0F,
                                  0.0F,
                                  static_cast<float>(frame.stageSize.width),
                                  static_cast<float>(frame.stageSize.height),
                                  0.0F,
                                  1.0F};
    frame.context->OMSetBlendState(blend, nullptr, 0xFFFFFFFFU);
    drawRange(frame.context, viewport, range, frame.stageSize);
}

void UiPass::copyScene(const FrameContext& frame)
{
    ensureSceneTexture(frame.stageSize);

    gpu::ComPtr<ID3D11Resource> source;
    frame.target->GetResource(&source);
    const D3D11_BOX box{
        0, 0, 0, static_cast<UINT>(frame.stageSize.width), static_cast<UINT>(frame.stageSize.height), 1};
    frame.context->CopySubresourceRegion(sceneTexture_.Get(), 0, 0, 0, 0, source.Get(), 0, &box);
}

void UiPass::uploadBackdropField(const FrameContext& frame)
{
    const BackdropField& field = frame.drawList->backdropField();

    BackdropConstants values{};
    values.position[0] = field.position.x;
    values.position[1] = field.position.y;
    values.velocity[0] = field.velocity.x;
    values.velocity[1] = field.velocity.y;
    values.trail[0] = field.trail.x;
    values.trail[1] = field.trail.y;
    values.presence = field.presence;
    values.radius = field.radius;
    for (std::size_t i = 0; i < field.ripples.size(); ++i)
    {
        const Ripple& ripple = field.ripples[i];
        values.ripples[i][0] = ripple.origin.x;
        values.ripples[i][1] = ripple.origin.y;
        values.ripples[i][2] = ripple.age;
        values.ripples[i][3] = ripple.amplitude;
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    platform::throwIfFailed(
        frame.context->Map(backdropConstants_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
        "Map(backdrop constants)");
    std::memcpy(mapped.pData, &values, sizeof(values));
    frame.context->Unmap(backdropConstants_.Get(), 0);
}

void UiPass::execute(const FrameContext& frame)
{
    ID3D11DeviceContext* context = frame.context;

    atlas_.sync(context);
    const UploadResult upload = uploadInstances(frame);
    uploadBackdropField(frame);

    ID3D11RenderTargetView* target = frame.target;
    context->OMSetRenderTargets(1, &target, nullptr);
    context->RSSetState(rasterizerState_.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context->PSSetShader(pixelShader_.Get(), nullptr, 0);
    ID3D11Buffer* constants = constants_.Get();
    context->VSSetConstantBuffers(0, 1, &constants);
    context->PSSetConstantBuffers(0, 1, &constants);
    ID3D11Buffer* backdropConstants = backdropConstants_.Get();
    context->PSSetConstantBuffers(1, 1, &backdropConstants);
    ID3D11ShaderResourceView* instancesView = instancesView_.Get();
    context->VSSetShaderResources(0, 1, &instancesView);
    context->PSSetShaderResources(0, 1, &instancesView);
    ID3D11ShaderResourceView* atlasView = atlas_.view();
    context->PSSetShaderResources(1, 1, &atlasView);
    ID3D11ShaderResourceView* artView = artView_.Get();
    context->PSSetShaderResources(3, 1, &artView);
    ID3D11ShaderResourceView* flagView = flagView_.Get();
    context->PSSetShaderResources(4, 1, &flagView);
    ID3D11SamplerState* sampler = glyphSampler_.Get();
    context->PSSetSamplers(0, 1, &sampler);

    for (std::size_t i = 0; i < kLayerCount; ++i)
    {
        const LayerRange& range = upload.layers[i];
        if (!isGlassLayer(static_cast<Layer>(i)))
        {
            drawLayer(frame, range, overBlend_.Get());
            continue;
        }
        if (range.count == 0)
            continue;

        copyScene(frame);
        ID3D11ShaderResourceView* sceneView = sceneView_.Get();
        context->PSSetShaderResources(2, 1, &sceneView);
        drawLayer(frame, range, replaceBlend_.Get());
        ID3D11ShaderResourceView* noScene = nullptr;
        context->PSSetShaderResources(2, 1, &noScene);
    }

    if (upload.mask.count > 0)
    {
        const float width = static_cast<float>(frame.stageSize.width);
        const float height = static_cast<float>(frame.stageSize.height);
        const D3D11_VIEWPORT maskViewport{width, 0.0F, width, height, 0.0F, 1.0F};
        context->OMSetBlendState(overBlend_.Get(), nullptr, 0xFFFFFFFFU);
        drawRange(context, maskViewport, upload.mask, frame.stageSize);
    }

    ID3D11ShaderResourceView* noView = nullptr;
    context->VSSetShaderResources(0, 1, &noView);
    context->PSSetShaderResources(0, 1, &noView);
    context->PSSetShaderResources(1, 1, &noView);
    context->PSSetShaderResources(3, 1, &noView);
    context->PSSetShaderResources(4, 1, &noView);
}

}
