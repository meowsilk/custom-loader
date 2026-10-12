#pragma once

#include "core/secret_string.hxx"
#include "text/shaper.hxx"
#include "ui/animation/animated.hxx"
#include "ui/localization.hxx"
#include "ui/text_edit/text_editor.hxx"
#include "ui/widget.hxx"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace loader::ui
{

class TextField : public Widget
{
public:
    enum class Kind
    {
        Plain,
        Secret,
    };

    TextField(UiContext& context, Kind kind, char32_t leadingIcon, std::u32string placeholder);

    Kind kind() const { return kind_; }
    bool secret() const { return kind_ == Kind::Secret; }

    std::u32string_view text() const { return editor_.text(); }
    bool empty() const { return editor_.empty() && !savedSecret_; }
    void setText(std::u32string_view text);
    void clear();

    void setPlaceholder(std::u32string placeholder);
    void bindPlaceholder(Str id);

    void setSavedSecret(bool present, const core::SecretString* source = nullptr);
    bool hasSavedSecret() const { return savedSecret_; }
    bool revealing() const { return reveal_.target() > 0.5F; }

    void setOnChanged(std::function<void()> handler) { onChanged_ = std::move(handler); }
    void setOnSubmit(std::function<void()> handler) { onSubmit_ = std::move(handler); }
    void setOnBlur(std::function<void()> handler) { onBlur_ = std::move(handler); }
    void setError(bool error);
    void setReadOnly(bool readOnly);
    void setTransform(const text_edit::TextTransform* transform) { editor_.setTransform(transform); }

    core::Rect caretRect() const;

    bool onPointerDown(core::Vec2 position, core::input::PointerButton button) override;
    void onPointerMove(core::Vec2 position) override;
    void onPointerUp(core::Vec2 position, core::input::PointerButton button) override;
    void onHoverMove(core::Vec2 position) override;
    bool onKeyDown(const core::input::KeyEvent& event) override;
    void onChar(char32_t codePoint) override;
    core::input::Cursor cursorKind() const override { return core::input::Cursor::IBeam; }

protected:
    core::Size measureContent() override;
    void paintSelf(PaintContext& context) override;
    bool updateSelf(float deltaSeconds) override;
    std::optional<float> idleWakeSeconds() const override;
    bool hitTestable() const override { return true; }
    void onStateChanged() override;

private:
    struct Layout
    {
        core::Rect textArea;
        core::Rect tile;
    };

    Layout computeLayout() const;
    core::Rect eyeZone() const;
    const text::TextStyle& textStyle() const;
    text::ShapedText shapeVisibleText() const;
    std::vector<float> displayOffsets() const;
    float hiddenAdvance() const;
    std::size_t hiddenCount() const;
    void ensureCaretVisible(const std::vector<float>& offsets, float areaWidth);
    void restartBlink();
    void dropSavedSecret();
    bool canReveal() const;
    std::u32string_view visibleSource() const;
    void afterEdit(unsigned previousRevision);
    void paintHiddenDots(PaintContext& context, const core::Rect& area, float opacity);
    void paintVisibleText(PaintContext& context, const core::Rect& area, float opacity);

    const core::SecretString* savedSource_ = nullptr;
    TextBinding placeholderBinding_;
    Kind kind_;
    char32_t leadingIcon_;
    std::u32string placeholder_;
    text_edit::TextEditor editor_;
    text::ShapedText iconShape_;
    text::ShapedText eyeShape_;
    text::ShapedText placeholderShape_;
    std::function<void()> onChanged_;
    std::function<void()> onSubmit_;
    std::function<void()> onBlur_;
    animation::Animated focus_;
    animation::Animated error_;
    animation::Animated reveal_;
    animation::Animated hover_;
    animation::Animated eyeHover_;
    float scrollX_ = 0.0F;
    float blinkTimer_ = 0.0F;
    bool caretVisible_ = true;
    bool savedSecret_ = false;
    bool readOnly_ = false;
    bool dragging_ = false;
    bool wasFocused_ = false;
    bool eyeHot_ = false;
};

}
