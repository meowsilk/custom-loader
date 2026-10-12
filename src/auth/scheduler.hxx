#pragma once

#include <chrono>
#include <cstdint>
#include <functional>

namespace loader::auth
{

using TimerId = std::uint64_t;

class Scheduler
{
public:
    virtual ~Scheduler() = default;

    virtual TimerId post(std::chrono::milliseconds delay, std::function<void()> task) = 0;
    virtual void cancel(TimerId id) = 0;
};

}
