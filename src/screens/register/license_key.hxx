#pragma once

#include "ui/text_edit/text_editor.hxx"

#include <string>
#include <string_view>

namespace loader::screens
{

std::u32string normalizeLicenseKey(std::u32string_view raw);
std::u32string formatLicenseKey(std::u32string_view normalized);

class LicenseKeyTransform final : public ui::text_edit::TextTransform
{
public:
    std::u32string apply(std::u32string_view text, std::size_t& caret) const override;
};

}
