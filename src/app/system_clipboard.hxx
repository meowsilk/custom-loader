#pragma once

#include "ui/ui_context.hxx"

#include <Windows.h>

namespace loader::app
{

class SystemClipboard final : public ui::Clipboard
{
public:
    explicit SystemClipboard(HWND owner) : owner_(owner) {}

    std::u32string text() override;
    void setText(std::u32string_view text) override;

private:
    HWND owner_;
};

}
