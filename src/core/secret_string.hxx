#pragma once

#include "core/secure_memory.hxx"

#include <string_view>
#include <utility>

namespace loader::core
{

class SecretString
{
public:
    SecretString() = default;
    explicit SecretString(std::u32string_view text) : value_(text) {}

    SecretString(const SecretString&) = delete;
    SecretString& operator=(const SecretString&) = delete;

    SecretString(SecretString&& other) noexcept : value_(std::move(other.value_)) { other.wipe(); }
    SecretString& operator=(SecretString&& other) noexcept
    {
        if (this != &other)
        {
            wipe();
            value_ = std::move(other.value_);
            other.wipe();
        }
        return *this;
    }

    ~SecretString() { wipe(); }

    std::u32string_view view() const { return value_; }
    bool empty() const { return value_.empty(); }
    std::size_t size() const { return value_.size(); }

    void assign(std::u32string_view text)
    {
        wipe();
        value_.assign(text);
    }

    void wipe()
    {
        if (!value_.empty())
            SecureZeroMemory(value_.data(), value_.size() * sizeof(char32_t));
        value_.clear();
    }

private:
    SecureString32 value_;
};

}
