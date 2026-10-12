#include "app/ui_scheduler.hxx"

#include <algorithm>

namespace loader::app
{

auth::TimerId UiScheduler::post(std::chrono::milliseconds delay, std::function<void()> task)
{
    const auth::TimerId id = nextId_++;
    entries_.push_back({id, Clock::now() + delay, std::move(task)});
    return id;
}

void UiScheduler::cancel(auth::TimerId id)
{
    std::erase_if(entries_, [id](const Entry& entry) { return entry.id == id; });
}

void UiScheduler::runDue()
{
    const Clock::time_point now = Clock::now();

    std::vector<Entry> due;
    for (auto it = entries_.begin(); it != entries_.end();)
    {
        if (it->due <= now)
        {
            due.push_back(std::move(*it));
            it = entries_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    std::ranges::sort(due, {}, &Entry::due);
    for (Entry& entry : due)
        entry.task();
}

std::optional<std::chrono::milliseconds> UiScheduler::timeUntilNext() const
{
    if (entries_.empty())
        return std::nullopt;

    const auto earliest = std::ranges::min_element(entries_, {}, &Entry::due);
    const auto remaining =
        std::chrono::duration_cast<std::chrono::milliseconds>(earliest->due - Clock::now());
    return std::max(remaining, std::chrono::milliseconds(0));
}

}
