#pragma once

#include "core/secure_memory.hxx"

#include <cstddef>
#include <string>
#include <string_view>

namespace loader::ui::text_edit
{

class TextStorage
{
public:
    virtual ~TextStorage() = default;

    virtual std::u32string_view view() const = 0;
    virtual void insert(std::size_t position, std::u32string_view text) = 0;
    virtual void erase(std::size_t position, std::size_t count) = 0;
    virtual void assign(std::u32string_view text) = 0;
    virtual void clear() = 0;
};

class PlainTextStorage final : public TextStorage
{
public:
    std::u32string_view view() const override { return text_; }
    void insert(std::size_t position, std::u32string_view text) override { text_.insert(position, text); }
    void erase(std::size_t position, std::size_t count) override { text_.erase(position, count); }
    void assign(std::u32string_view text) override { text_.assign(text); }
    void clear() override { text_.clear(); }

private:
    std::u32string text_;
};

class SecureTextStorage final : public TextStorage
{
public:
    static constexpr std::size_t kReservedCharacters = 256;

    SecureTextStorage() { text_.reserve(kReservedCharacters); }
    ~SecureTextStorage() override { clear(); }

    std::u32string_view view() const override { return text_; }
    void insert(std::size_t position, std::u32string_view text) override { text_.insert(position, text); }
    void erase(std::size_t position, std::size_t count) override { text_.erase(position, count); }
    void assign(std::u32string_view text) override { text_.assign(text); }

    void clear() override
    {
        SecureZeroMemory(text_.data(), text_.size() * sizeof(char32_t));
        text_.clear();
    }

private:
    core::SecureString32 text_;
};

}
