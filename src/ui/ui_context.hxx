#pragma once

#include "text/font_library.hxx"
#include "ui/localization.hxx"
#include "ui/theme/theme.hxx"

#include <algorithm>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace loader::ui
{

class Widget;

class Clipboard
{
public:
    virtual ~Clipboard() = default;

    virtual std::u32string text() = 0;
    virtual void setText(std::u32string_view text) = 0;
};

class UiContext
{
public:
    UiContext(const theme::Theme& theme,
              text::FontLibrary& fonts,
              Clipboard& clipboard,
              std::function<void()> requestFrame)
        : theme_(theme), fonts_(fonts), clipboard_(clipboard), requestFrame_(std::move(requestFrame))
    {
    }

    const theme::Theme& theme() const { return theme_; }
    text::FontLibrary& fonts() const { return fonts_; }
    Localization& localization() const { return localization_; }
    Clipboard& clipboard() const { return clipboard_; }
    class Observation
    {
    public:
        Observation() = default;
        Observation(UiContext& owner, std::size_t id) : owner_(&owner), id_(id) {}
        ~Observation() { release(); }

        Observation(const Observation&) = delete;
        Observation& operator=(const Observation&) = delete;
        Observation(Observation&& other) noexcept
            : owner_(std::exchange(other.owner_, nullptr)), id_(other.id_)
        {
        }
        Observation& operator=(Observation&& other) noexcept
        {
            if (this != &other)
            {
                release();
                owner_ = std::exchange(other.owner_, nullptr);
                id_ = other.id_;
            }
            return *this;
        }

    private:
        void release()
        {
            if (owner_ != nullptr)
                owner_->stopObserving(id_);
            owner_ = nullptr;
        }

        UiContext* owner_ = nullptr;
        std::size_t id_ = 0;
    };

    [[nodiscard]] Observation observeDestruction(std::function<void(const Widget*)> listener)
    {
        const std::size_t id = nextObserverId_++;
        destroyListeners_.emplace_back(id, std::move(listener));
        return Observation(*this, id);
    }

    void notifyDestroyed(const Widget* widget) const
    {
        const auto snapshot = destroyListeners_;
        for (const auto& entry : snapshot)
            entry.second(widget);
    }

    void requestFrame() const
    {
        if (requestFrame_)
            requestFrame_();
    }

private:
    const theme::Theme& theme_;
    text::FontLibrary& fonts_;
    Clipboard& clipboard_;
    std::function<void()> requestFrame_;
    void stopObserving(std::size_t id)
    {
        std::erase_if(destroyListeners_, [id](const auto& entry) { return entry.first == id; });
    }

    mutable Localization localization_;
    mutable std::vector<std::pair<std::size_t, std::function<void(const Widget*)>>> destroyListeners_;
    std::size_t nextObserverId_ = 1;
};

}
