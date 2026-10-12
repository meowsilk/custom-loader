#include "platform/image_decoder.hxx"

#include "gpu/com_ptr.hxx"
#include "platform/win32_error.hxx"

#include <wincodec.h>

#include <Windows.h>

#include <cstring>

namespace loader::platform
{
namespace
{

class ComScope
{
public:
    ComScope() : result_(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}
    ~ComScope()
    {
        if (SUCCEEDED(result_))
            CoUninitialize();
    }

    ComScope(const ComScope&) = delete;
    ComScope& operator=(const ComScope&) = delete;

private:
    HRESULT result_;
};

}

DecodedImage decodeImage(std::span<const std::byte> encoded)
{
    const ComScope com;

    gpu::ComPtr<IWICImagingFactory> factory;
    throwIfFailed(
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)),
        "CoCreateInstance(WICImagingFactory)");

    std::vector<BYTE> owned(encoded.size());
    std::memcpy(owned.data(), encoded.data(), encoded.size());
    gpu::ComPtr<IWICStream> stream;
    throwIfFailed(factory->CreateStream(&stream), "IWICImagingFactory::CreateStream");
    throwIfFailed(stream->InitializeFromMemory(owned.data(), static_cast<DWORD>(owned.size())),
                  "IWICStream::InitializeFromMemory");

    gpu::ComPtr<IWICBitmapDecoder> decoder;
    throwIfFailed(
        factory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder),
        "IWICImagingFactory::CreateDecoderFromStream");

    gpu::ComPtr<IWICBitmapFrameDecode> frame;
    throwIfFailed(decoder->GetFrame(0, &frame), "IWICBitmapDecoder::GetFrame");

    gpu::ComPtr<IWICFormatConverter> converter;
    throwIfFailed(factory->CreateFormatConverter(&converter), "IWICImagingFactory::CreateFormatConverter");
    throwIfFailed(converter->Initialize(frame.Get(),
                                        GUID_WICPixelFormat32bppPBGRA,
                                        WICBitmapDitherTypeNone,
                                        nullptr,
                                        0.0,
                                        WICBitmapPaletteTypeCustom),
                  "IWICFormatConverter::Initialize");

    UINT width = 0;
    UINT height = 0;
    throwIfFailed(converter->GetSize(&width, &height), "IWICFormatConverter::GetSize");

    DecodedImage image;
    image.size = {static_cast<std::int32_t>(width), static_cast<std::int32_t>(height)};
    image.bgra.resize(static_cast<std::size_t>(width) * height * 4U);
    throwIfFailed(
        converter->CopyPixels(nullptr, width * 4U, static_cast<UINT>(image.bgra.size()), image.bgra.data()),
        "IWICFormatConverter::CopyPixels");
    return image;
}

}
