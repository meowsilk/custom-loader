#include "ui/text_edit/text_editor.hxx"

#include <algorithm>

namespace loader::ui::text_edit
{
namespace
{

constexpr std::size_t kMaxUndoDepth = 64;

bool isWordCharacter(char32_t character)
{
    return character == U'_' || (character >= U'0' && character <= U'9') ||
           (character >= U'a' && character <= U'z') || (character >= U'A' && character <= U'Z') ||
           character > 0x7F;
}

std::u32string stripLineBreaks(std::u32string_view text)
{
    std::u32string result;
    result.reserve(text.size());
    for (const char32_t character : text)
    {
        if (character >= 0x20 && character != 0x7F)
            result.push_back(character);
    }
    return result;
}

}

TextEditor::TextEditor(std::unique_ptr<TextStorage> storage, EditorOptions options)
    : storage_(std::move(storage)), options_(options)
{
}

std::pair<std::size_t, std::size_t> TextEditor::selection() const
{
    return {std::min(caret_, anchor_), std::max(caret_, anchor_)};
}

void TextEditor::recordUndo()
{
    if (!options_.keepUndoHistory)
        return;
    undo_.push_back({std::u32string(storage_->view()), caret_});
    if (undo_.size() > kMaxUndoDepth)
        undo_.erase(undo_.begin());
    redo_.clear();
}

void TextEditor::restore(const Snapshot& snapshot)
{
    storage_->assign(snapshot.text);
    caret_ = anchor_ = std::min(snapshot.caret, snapshot.text.size());
    touch();
}

void TextEditor::applyTransform()
{
    if (transform_ == nullptr)
        return;

    std::size_t caret = caret_;
    const std::u32string formatted = transform_->apply(storage_->view(), caret);
    if (formatted != storage_->view())
        storage_->assign(formatted);
    caret_ = anchor_ = std::min(caret, storage_->view().size());
}

void TextEditor::deleteSelection()
{
    const auto [begin, end] = selection();
    if (begin == end)
        return;
    storage_->erase(begin, end - begin);
    caret_ = anchor_ = begin;
}

void TextEditor::insert(std::u32string_view text)
{
    const std::u32string clean = stripLineBreaks(text);
    if (clean.empty() && !hasSelection())
        return;

    recordUndo();
    deleteSelection();

    const std::size_t room = options_.maxLength > length() ? options_.maxLength - length() : 0;
    const std::u32string_view accepted = std::u32string_view(clean).substr(0, room);
    storage_->insert(caret_, accepted);
    caret_ = anchor_ = caret_ + accepted.size();

    applyTransform();
    touch();
}

void TextEditor::setText(std::u32string_view text)
{
    storage_->assign(text.substr(0, options_.maxLength));
    caret_ = anchor_ = storage_->view().size();
    undo_.clear();
    redo_.clear();
    applyTransform();
    touch();
}

void TextEditor::clear()
{
    storage_->clear();
    caret_ = anchor_ = 0;
    undo_.clear();
    redo_.clear();
    touch();
}

void TextEditor::selectAll()
{
    anchor_ = 0;
    caret_ = length();
}

void TextEditor::moveCaret(std::size_t position, bool extendSelection)
{
    caret_ = std::min(position, length());
    if (!extendSelection)
        anchor_ = caret_;
}

std::size_t TextEditor::wordBoundary(std::size_t from, bool forward) const
{
    const std::u32string_view view = storage_->view();
    std::size_t position = from;

    if (forward)
    {
        while (position < view.size() && !isWordCharacter(view[position]))
            ++position;
        while (position < view.size() && isWordCharacter(view[position]))
            ++position;
    }
    else
    {
        while (position > 0 && !isWordCharacter(view[position - 1]))
            --position;
        while (position > 0 && isWordCharacter(view[position - 1]))
            --position;
    }
    return position;
}

void TextEditor::deleteBackward(bool byWord)
{
    if (hasSelection())
    {
        recordUndo();
        deleteSelection();
        applyTransform();
        touch();
        return;
    }
    if (caret_ == 0)
        return;

    recordUndo();
    const std::size_t target = byWord ? wordBoundary(caret_, false) : caret_ - 1;
    storage_->erase(target, caret_ - target);
    caret_ = anchor_ = target;
    applyTransform();
    touch();
}

void TextEditor::deleteForward(bool byWord)
{
    if (hasSelection())
    {
        recordUndo();
        deleteSelection();
        applyTransform();
        touch();
        return;
    }
    if (caret_ >= length())
        return;

    recordUndo();
    const std::size_t target = byWord ? wordBoundary(caret_, true) : caret_ + 1;
    storage_->erase(caret_, target - caret_);
    applyTransform();
    touch();
}

bool TextEditor::handleKey(const core::input::KeyEvent& event, Clipboard& clipboard)
{
    using core::input::Key;

    const bool control = event.modifiers.control;
    const bool shift = event.modifiers.shift;

    switch (event.key)
    {
    case Key::Left:
        if (hasSelection() && !shift)
            moveCaret(selection().first, false);
        else
            moveCaret(control ? wordBoundary(caret_, false) : (caret_ > 0 ? caret_ - 1 : 0), shift);
        return true;
    case Key::Right:
        if (hasSelection() && !shift)
            moveCaret(selection().second, false);
        else
            moveCaret(control ? wordBoundary(caret_, true) : caret_ + 1, shift);
        return true;
    case Key::Home:
        moveCaret(0, shift);
        return true;
    case Key::End:
        moveCaret(length(), shift);
        return true;
    case Key::Backspace:
        deleteBackward(control);
        return true;
    case Key::Delete:
        deleteForward(control);
        return true;
    case Key::A:
        if (!control)
            return false;
        selectAll();
        return true;
    case Key::C:
    case Key::X:
    {
        if (!control)
            return false;
        if (options_.allowClipboardWrite && hasSelection())
        {
            const auto [begin, end] = selection();
            clipboard.setText(storage_->view().substr(begin, end - begin));
            if (event.key == Key::X)
                deleteBackward(false);
        }
        return true;
    }
    case Key::V:
        if (!control)
            return false;
        if (options_.allowClipboardRead)
            insert(clipboard.text());
        return true;
    case Key::Z:
    {
        if (!control || !options_.keepUndoHistory)
            return false;
        if (undo_.empty())
            return true;
        redo_.push_back({std::u32string(storage_->view()), caret_});
        const Snapshot snapshot = std::move(undo_.back());
        undo_.pop_back();
        restore(snapshot);
        return true;
    }
    case Key::Y:
    {
        if (!control || !options_.keepUndoHistory)
            return false;
        if (redo_.empty())
            return true;
        undo_.push_back({std::u32string(storage_->view()), caret_});
        const Snapshot snapshot = std::move(redo_.back());
        redo_.pop_back();
        restore(snapshot);
        return true;
    }
    default:
        return false;
    }
}

}
