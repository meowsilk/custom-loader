#include "platform/overlay_window.hxx"

#include "platform/dpi.hxx"
#include "platform/keyboard.hxx"
#include "platform/win32_error.hxx"

#include <imm.h>

#include <windowsx.h>

#include <algorithm>
#include <cmath>

namespace loader::platform
{
namespace
{

constexpr wchar_t kInputClass[] = L"loader.gg.input";
constexpr wchar_t kStageClass[] = L"loader.gg.stage";

void registerWindowClass(const wchar_t* name, WNDPROC procedure)
{
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = procedure;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = name;
    if (RegisterClassExW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        throwLastError("RegisterClassExW");
}

core::SizeI toPixels(core::Size logical, float scale)
{
    return {static_cast<std::int32_t>(std::lround(logical.width * scale)),
            static_cast<std::int32_t>(std::lround(logical.height * scale))};
}

RECT workAreaAt(POINT point)
{
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    GetMonitorInfoW(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &info);
    return info.rcWork;
}

LONG clampAxis(LONG value, LONG low, LONG high)
{
    return std::clamp(value, low, std::max(low, high));
}

}

OverlayWindow::OverlayWindow(WindowEvents& events, const OverlayDesc& desc)
    : events_(events), stageLogical_(desc.stageSize), cornerRadius_(desc.panelCornerRadius)
{
    registerWindowClass(kInputClass, &OverlayWindow::inputProc);
    registerWindowClass(kStageClass, &OverlayWindow::stageProc);

    POINT cursor{};
    GetCursorPos(&cursor);
    const HMONITOR monitor = MonitorFromPoint(cursor, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfoW(monitor, &monitorInfo);

    scale_ = scaleFromDpi(dpiForMonitor(monitor));
    stagePixels_ = toPixels(stageLogical_, scale_);

    const RECT& work = monitorInfo.rcWork;
    const int originX = work.left + ((work.right - work.left) - stagePixels_.width) / 2;
    const int originY = work.top + ((work.bottom - work.top) - stagePixels_.height) / 2;

    panelLogical_ = {(stageLogical_.width - desc.initialPanelSize.width) * 0.5F,
                     (stageLogical_.height - desc.initialPanelSize.height) * 0.5F,
                     desc.initialPanelSize.width,
                     desc.initialPanelSize.height};

    const HINSTANCE instance = GetModuleHandleW(nullptr);

    input_ = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_APPWINDOW,
                             kInputClass,
                             desc.title,
                             WS_POPUP | WS_MINIMIZEBOX,
                             originX,
                             originY,
                             stagePixels_.width,
                             stagePixels_.height,
                             nullptr,
                             nullptr,
                             instance,
                             this);
    if (input_ == nullptr)
        throwLastError("CreateWindowExW(input)");

    stage_ = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT |
                                 WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                             kStageClass,
                             L"",
                             WS_POPUP,
                             originX,
                             originY,
                             stagePixels_.width,
                             stagePixels_.height,
                             input_,
                             nullptr,
                             instance,
                             this);
    if (stage_ == nullptr)
        throwLastError("CreateWindowExW(stage)");

    SetLayeredWindowAttributes(stage_, 0, 255, LWA_ALPHA);
    applyRegion();
}

OverlayWindow::~OverlayWindow()
{
    if (stage_ != nullptr)
        DestroyWindow(stage_);
    if (input_ != nullptr)
        DestroyWindow(input_);
}

core::Rect OverlayWindow::panelRectPixels() const
{
    return {panelLogical_.x * scale_,
            panelLogical_.y * scale_,
            panelLogical_.width * scale_,
            panelLogical_.height * scale_};
}

core::Size OverlayWindow::workAreaLogical() const
{
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    GetMonitorInfoW(MonitorFromWindow(input_, MONITOR_DEFAULTTONEAREST), &info);
    return {static_cast<float>(info.rcWork.right - info.rcWork.left) / scale_,
            static_cast<float>(info.rcWork.bottom - info.rcWork.top) / scale_};
}

void OverlayWindow::setPanel(core::Rect logicalRect, float cornerRadius)
{
    panelLogical_ = logicalRect;
    cornerRadius_ = cornerRadius;
    applyRegion();

    if (drag_)
        return;

    const POINT origin = currentOrigin();
    const core::Rect panel = panelRectPixels();
    const POINT center{origin.x + static_cast<LONG>(std::lround(panel.center().x)),
                       origin.y + static_cast<LONG>(std::lround(panel.center().y))};
    const POINT clamped = clampToWorkArea(origin, center);
    if (clamped.x != origin.x || clamped.y != origin.y)
        applyGeometry(clamped);
}

void OverlayWindow::show()
{
    ShowWindow(stage_, SW_SHOWNOACTIVATE);
    ShowWindow(input_, SW_SHOW);
    SetWindowPos(input_, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetForegroundWindow(input_);
}

void OverlayWindow::minimize()
{
    ShowWindow(input_, SW_MINIMIZE);
}

void OverlayWindow::setImeState(bool enabled, core::Rect caretStagePixels)
{
    imeCaret_ = caretStagePixels;
    if (imeEnabled_ != enabled)
    {
        imeEnabled_ = enabled;
        if (enabled)
            ImmAssociateContextEx(input_, nullptr, IACE_DEFAULT);
        else
            ImmAssociateContext(input_, nullptr);
    }
    applyImePosition();
}

void OverlayWindow::applyImePosition()
{
    if (!imeEnabled_)
        return;

    const HIMC context = ImmGetContext(input_);
    if (context == nullptr)
        return;

    const POINT position{static_cast<LONG>(std::lround(imeCaret_.x)),
                         static_cast<LONG>(std::lround(imeCaret_.y))};
    COMPOSITIONFORM composition{CFS_POINT, position, {}};
    ImmSetCompositionWindow(context, &composition);
    CANDIDATEFORM candidate{
        0, CFS_CANDIDATEPOS, {position.x, position.y + static_cast<LONG>(std::lround(imeCaret_.height))}, {}};
    ImmSetCandidateWindow(context, &candidate);
    ImmReleaseContext(input_, context);
}

void OverlayWindow::applyRegion()
{
    const core::Rect panel = panelRectPixels();
    const int left = static_cast<int>(std::lround(panel.x));
    const int top = static_cast<int>(std::lround(panel.y));
    const int right = static_cast<int>(std::lround(panel.right()));
    const int bottom = static_cast<int>(std::lround(panel.bottom()));
    const int diameter = static_cast<int>(std::lround(cornerRadius_ * scale_ * 2.0F));

    const HRGN region = CreateRoundRectRgn(left, top, right + 1, bottom + 1, diameter, diameter);
    if (SetWindowRgn(input_, region, FALSE) == 0)
        DeleteObject(region);
}

POINT OverlayWindow::currentOrigin() const
{
    RECT rect{};
    GetWindowRect(input_, &rect);
    return {rect.left, rect.top};
}

void OverlayWindow::applyGeometry(POINT origin)
{
    HDWP batch = BeginDeferWindowPos(2);
    constexpr UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;
    batch = DeferWindowPos(
        batch, input_, nullptr, origin.x, origin.y, stagePixels_.width, stagePixels_.height, flags);
    batch = DeferWindowPos(
        batch, stage_, nullptr, origin.x, origin.y, stagePixels_.width, stagePixels_.height, flags);
    if (batch != nullptr)
        EndDeferWindowPos(batch);
}

POINT OverlayWindow::clampToWorkArea(POINT origin, POINT reference) const
{
    const RECT work = workAreaAt(reference);
    const core::Rect panel = panelRectPixels();
    const auto offset = [](float value) { return static_cast<LONG>(std::lround(value)); };
    return {clampAxis(origin.x, work.left - offset(panel.x), work.right - offset(panel.right())),
            clampAxis(origin.y, work.top - offset(panel.y), work.bottom - offset(panel.bottom()))};
}

core::Vec2 OverlayWindow::toStagePoint(LPARAM lparam) const
{
    return {static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam))};
}

void OverlayWindow::handleChar(WPARAM unit)
{
    const auto value = static_cast<wchar_t>(unit);
    if (value >= 0xD800 && value <= 0xDBFF)
    {
        pendingHighSurrogate_ = value;
        return;
    }

    char32_t codePoint = value;
    if (value >= 0xDC00 && value <= 0xDFFF)
    {
        if (pendingHighSurrogate_ == 0)
            return;
        codePoint = 0x10000 + ((static_cast<char32_t>(pendingHighSurrogate_) - 0xD800) << 10) +
                    (static_cast<char32_t>(value) - 0xDC00);
    }
    pendingHighSurrogate_ = 0;

    if (codePoint < 0x20 || codePoint == 0x7F)
        return;
    events_.onChar(codePoint);
}

void OverlayWindow::beginDrag(POINT cursor)
{
    const POINT origin = currentOrigin();
    const core::Rect panel = panelRectPixels();
    drag_ = DragState{cursor,
                      origin,
                      {(static_cast<float>(cursor.x - origin.x) - panel.x) / scale_,
                       (static_cast<float>(cursor.y - origin.y) - panel.y) / scale_}};
    SetCapture(input_);
}

void OverlayWindow::endDrag()
{
    if (drag_)
    {
        drag_.reset();
        if (GetCapture() == input_)
            ReleaseCapture();
    }
}

void OverlayWindow::dragTo(POINT cursor)
{
    if (!drag_)
        return;
    const POINT desired{drag_->startOrigin.x + (cursor.x - drag_->startCursor.x),
                        drag_->startOrigin.y + (cursor.y - drag_->startCursor.y)};
    applyGeometry(clampToWorkArea(desired, cursor));
}

void OverlayWindow::handleDpiChanged(unsigned dpi)
{
    const float newScale = scaleFromDpi(dpi);
    if (newScale == scale_)
        return;

    POINT cursor{};
    GetCursorPos(&cursor);

    const POINT oldOrigin = currentOrigin();
    const core::Rect oldPanel = panelRectPixels();
    const core::Vec2 oldCenter{static_cast<float>(oldOrigin.x) + oldPanel.center().x,
                               static_cast<float>(oldOrigin.y) + oldPanel.center().y};

    scale_ = newScale;
    stagePixels_ = toPixels(stageLogical_, scale_);
    const core::Rect newPanel = panelRectPixels();

    POINT newOrigin{};
    if (drag_)
    {
        newOrigin.x = static_cast<LONG>(
            std::lround(static_cast<float>(cursor.x) - newPanel.x - drag_->grabLogical.x * scale_));
        newOrigin.y = static_cast<LONG>(
            std::lround(static_cast<float>(cursor.y) - newPanel.y - drag_->grabLogical.y * scale_));
    }
    else
    {
        newOrigin.x = static_cast<LONG>(std::lround(oldCenter.x - newPanel.center().x));
        newOrigin.y = static_cast<LONG>(std::lround(oldCenter.y - newPanel.center().y));
    }

    newOrigin = clampToWorkArea(newOrigin, cursor);
    applyGeometry(newOrigin);
    applyRegion();

    if (drag_)
    {
        drag_->startCursor = cursor;
        drag_->startOrigin = newOrigin;
    }

    events_.onStageChanged();
}

void OverlayWindow::handlePointerMessage(UINT message, LPARAM lparam)
{
    const core::Vec2 point = toStagePoint(lparam);

    switch (message)
    {
    case WM_MOUSEMOVE:
    {
        if (!trackingLeave_)
        {
            TRACKMOUSEEVENT track{sizeof(track), TME_LEAVE, input_, 0};
            trackingLeave_ = TrackMouseEvent(&track) != FALSE;
        }
        if (drag_)
        {
            POINT cursor{};
            GetCursorPos(&cursor);
            dragTo(cursor);
        }
        else
        {
            events_.onPointerMoved(point);
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        if (events_.isCaption(point))
        {
            POINT cursor{};
            GetCursorPos(&cursor);
            beginDrag(cursor);
        }
        else
        {
            SetCapture(input_);
            events_.onPointerButton(core::input::PointerButton::Left, true, point);
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (drag_)
        {
            endDrag();
        }
        else
        {
            if (GetCapture() == input_)
                ReleaseCapture();
            events_.onPointerButton(core::input::PointerButton::Left, false, point);
        }
        break;
    }
    case WM_RBUTTONDOWN:
        events_.onPointerButton(core::input::PointerButton::Right, true, point);
        break;
    case WM_RBUTTONUP:
        events_.onPointerButton(core::input::PointerButton::Right, false, point);
        break;
    case WM_MBUTTONDOWN:
        events_.onPointerButton(core::input::PointerButton::Middle, true, point);
        break;
    case WM_MBUTTONUP:
        events_.onPointerButton(core::input::PointerButton::Middle, false, point);
        break;
    default:
        break;
    }
}

LRESULT OverlayWindow::handleInput(UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        handlePointerMessage(message, lparam);
        return 0;
    case WM_MOUSEWHEEL:
    {
        POINT cursor{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        ScreenToClient(input_, &cursor);
        events_.onWheel({static_cast<float>(cursor.x), static_cast<float>(cursor.y)},
                        static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<float>(WHEEL_DELTA));
        return 0;
    }
    case WM_MOUSELEAVE:
        trackingLeave_ = false;
        events_.onPointerLeft();
        return 0;
    case WM_CAPTURECHANGED:
        drag_.reset();
        return 0;
    case WM_KEYDOWN:
        events_.onKeyDown({keyFromVirtualKey(wparam), currentModifiers(), (lparam & (1 << 30)) != 0});
        return 0;
    case WM_SETCURSOR:
    {
        if (LOWORD(lparam) != HTCLIENT)
            return DefWindowProcW(input_, message, wparam, lparam);

        POINT cursor{};
        GetCursorPos(&cursor);
        ScreenToClient(input_, &cursor);
        const core::input::Cursor kind =
            events_.cursorAt({static_cast<float>(cursor.x), static_cast<float>(cursor.y)});
        LPCWSTR resource = kind == core::input::Cursor::IBeam  ? IDC_IBEAM
                           : kind == core::input::Cursor::Hand ? IDC_HAND
                                                               : IDC_ARROW;
        SetCursor(LoadCursorW(nullptr, resource));
        return TRUE;
    }
    case WM_CHAR:
        handleChar(wparam);
        return 0;
    case WM_IME_STARTCOMPOSITION:
        applyImePosition();
        return DefWindowProcW(input_, message, wparam, lparam);
    case WM_ACTIVATE:
        events_.onActivationChanged(LOWORD(wparam) != WA_INACTIVE);
        return 0;
    case WM_DPICHANGED:
        handleDpiChanged(LOWORD(wparam));
        return 0;
    case WM_SETTINGCHANGE:
        events_.onSystemSettingsChanged();
        return DefWindowProcW(input_, message, wparam, lparam);
    case WM_CLOSE:
        events_.onCloseRequested();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(input_, message, wparam, lparam);
    }
}

LRESULT CALLBACK OverlayWindow::inputProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_NCCREATE)
    {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    }

    auto* self = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (self == nullptr || self->input_ == nullptr)
        return DefWindowProcW(window, message, wparam, lparam);
    return self->handleInput(message, wparam, lparam);
}

LRESULT CALLBACK OverlayWindow::stageProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_NCHITTEST:
        return HTTRANSPARENT;
    case WM_DPICHANGED:
        return 0;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(window, message, wparam, lparam);
    }
}

}
