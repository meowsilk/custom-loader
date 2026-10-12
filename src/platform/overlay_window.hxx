#pragma once

#include "core/geometry.hxx"
#include "platform/window_events.hxx"

#include <Windows.h>

#include <optional>

namespace loader::platform
{

struct OverlayDesc
{
    core::Size stageSize;
    core::Size initialPanelSize;
    float panelCornerRadius = 0.0F;
    const wchar_t* title = L"";
};

class OverlayWindow
{
public:
    OverlayWindow(WindowEvents& events, const OverlayDesc& desc);
    ~OverlayWindow();

    OverlayWindow(const OverlayWindow&) = delete;
    OverlayWindow& operator=(const OverlayWindow&) = delete;

    HWND stageHandle() const { return stage_; }
    HWND inputHandle() const { return input_; }

    float scale() const { return scale_; }
    core::SizeI stagePixels() const { return stagePixels_; }
    core::Rect panelRectPixels() const;
    core::Size stageSizeLogical() const { return stageLogical_; }
    core::Size workAreaLogical() const;

    void setPanel(core::Rect logicalRect, float cornerRadius);
    void show();
    void minimize();
    void setImeState(bool enabled, core::Rect caretStagePixels);

private:
    static LRESULT CALLBACK inputProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK stageProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

    LRESULT handleInput(UINT message, WPARAM wparam, LPARAM lparam);
    void handlePointerMessage(UINT message, LPARAM lparam);
    void handleDpiChanged(unsigned dpi);
    void applyRegion();
    void applyGeometry(POINT origin);
    POINT currentOrigin() const;
    POINT clampToWorkArea(POINT origin, POINT reference) const;
    void handleChar(WPARAM unit);
    void applyImePosition();
    core::Vec2 toStagePoint(LPARAM lparam) const;
    void beginDrag(POINT cursor);
    void endDrag();
    void dragTo(POINT cursor);

    WindowEvents& events_;
    HWND input_ = nullptr;
    HWND stage_ = nullptr;

    core::Size stageLogical_;
    core::Rect panelLogical_;
    float cornerRadius_ = 0.0F;
    float scale_ = 1.0F;
    core::SizeI stagePixels_;

    struct DragState
    {
        POINT startCursor{};
        POINT startOrigin{};
        core::Vec2 grabLogical;
    };
    std::optional<DragState> drag_;
    bool trackingLeave_ = false;
    wchar_t pendingHighSurrogate_ = 0;
    bool imeEnabled_ = true;
    core::Rect imeCaret_;
};

}
