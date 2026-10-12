#include "app/system_clipboard.hxx"

#include "core/utf.hxx"

#include <cstring>

namespace loader::app
{
namespace
{

class ClipboardLock
{
public:
    explicit ClipboardLock(HWND owner) : open_(OpenClipboard(owner) != FALSE) {}
    ~ClipboardLock()
    {
        if (open_)
            CloseClipboard();
    }

    ClipboardLock(const ClipboardLock&) = delete;
    ClipboardLock& operator=(const ClipboardLock&) = delete;

    bool open() const { return open_; }

private:
    bool open_;
};

}

std::u32string SystemClipboard::text()
{
    const ClipboardLock lock(owner_);
    if (!lock.open() || !IsClipboardFormatAvailable(CF_UNICODETEXT))
        return {};

    const HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (data == nullptr)
        return {};

    const auto* characters = static_cast<const wchar_t*>(GlobalLock(data));
    if (characters == nullptr)
        return {};

    const std::u32string result = core::utf::decodeUtf16(std::wstring_view(characters));
    GlobalUnlock(data);
    return result;
}

void SystemClipboard::setText(std::u32string_view text)
{
    const std::wstring wide = core::utf::encodeUtf16(text);
    const std::size_t bytes = (wide.size() + 1) * sizeof(wchar_t);

    const HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (memory == nullptr)
        return;

    void* destination = GlobalLock(memory);
    if (destination == nullptr)
    {
        GlobalFree(memory);
        return;
    }
    std::memcpy(destination, wide.c_str(), bytes);
    GlobalUnlock(memory);

    const ClipboardLock lock(owner_);
    if (!lock.open())
    {
        GlobalFree(memory);
        return;
    }

    EmptyClipboard();
    if (SetClipboardData(CF_UNICODETEXT, memory) == nullptr)
        GlobalFree(memory);
}

}
