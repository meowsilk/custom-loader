#pragma once

#include "auth/scheduler.hxx"

#include <chrono>
#include <optional>
#include <vector>

namespace loader::app
{

class UiScheduler final : public auth::Scheduler
{
public:
    using Clock = std::chrono::steady_clock;

    auth::TimerId post(std::chrono::milliseconds delay, std::function<void()> task) override;
    void cancel(auth::TimerId id) override;

    void runDue();
    std::optional<std::chrono::milliseconds> timeUntilNext() const;

private:
    struct Entry
    {
        auth::TimerId id = 0;
        Clock::time_point due;
        std::function<void()> task;
    };

    std::vector<Entry> entries_;
    auth::TimerId nextId_ = 1;
};

}
