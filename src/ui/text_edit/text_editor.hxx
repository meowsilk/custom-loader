#pragma once

#include "core/input.hxx"
#include "ui/text_edit/text_storage.hxx"
#include "ui/ui_context.hxx"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace loader::ui::text_edit
{

class TextTransform
{
public:
    virtual ~TextTransform() = default;

    virtual std::u32string apply(std::u32string_view text, std::size_t& caret) const = 0;
};

struct EditorOptions
{
    bool allowClipboardRead = true;
    bool allowClipboardWrite = true;
    bool keepUndoHistory = true;
    std::size_t maxLength = 256;
};

class TextEditor
{
public:
    TextEditor(std::unique_ptr<TextStorage> storage, EditorOptions options);

    std::u32string_view text() const { return storage_->view(); }
    std::size_t length() const { return storage_->view().size(); }
    bool empty() const { return storage_->view().empty(); }
    std::size_t caret() const { return caret_; }
    std::size_t anchor() const { return anchor_; }
    bool hasSelection() const { return caret_ != anchor_; }
    std::pair<std::size_t, std::size_t> selection() const;
    unsigned revision() const { return revision_; }

    void setTransform(const TextTransform* transform) { transform_ = transform; }

    void insert(std::u32string_view text);
    void setText(std::u32string_view text);
    void clear();
    void selectAll();
    void moveCaret(std::size_t position, bool extendSelection);
    bool handleKey(const core::input::KeyEvent& event, Clipboard& clipboard);

private:
    struct Snapshot
    {
        std::u32string text;
        std::size_t caret = 0;
    };

    void deleteSelection();
    void deleteBackward(bool byWord);
    void deleteForward(bool byWord);
    void applyTransform();
    void recordUndo();
    void restore(const Snapshot& snapshot);
    std::size_t wordBoundary(std::size_t from, bool forward) const;
    void touch() { ++revision_; }

    std::unique_ptr<TextStorage> storage_;
    EditorOptions options_;
    const TextTransform* transform_ = nullptr;
    std::size_t caret_ = 0;
    std::size_t anchor_ = 0;
    unsigned revision_ = 0;
    std::vector<Snapshot> undo_;
    std::vector<Snapshot> redo_;
};

}
