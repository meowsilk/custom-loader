#pragma once

#include "core/geometry.hxx"
#include "core/input.hxx"

namespace loader::platform
{

class WindowEvents
{
public:
    virtual ~WindowEvents() = default;

    virtual bool isCaption(core::Vec2 stagePoint) const = 0;

    virtual core::input::Cursor cursorAt(core::Vec2) const { return core::input::Cursor::Arrow; }

    virtual void onStageChanged() {}
    virtual void onCloseRequested() {}
    virtual void onActivationChanged(bool) {}
    virtual void onSystemSettingsChanged() {}
    virtual void onPointerMoved(core::Vec2) {}
    virtual void onPointerLeft() {}
    virtual void onPointerButton(core::input::PointerButton, bool, core::Vec2) {}
    virtual void onWheel(core::Vec2, float) {}
    virtual void onKeyDown(const core::input::KeyEvent&) {}
    virtual void onChar(char32_t) {}
};

}
