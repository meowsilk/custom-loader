#pragma once

#include <Windows.h>

#include <chrono>
#include <optional>

namespace loader::platform
{

class FrameClient
{
public:
    virtual ~FrameClient() = default;

    virtual bool wantsFrame() const = 0;
    virtual HANDLE frameReadyEvent() const = 0;
    virtual void renderFrame() = 0;
    virtual std::optional<std::chrono::milliseconds> idleTimeout() const { return std::nullopt; }
    virtual void onIdleTimeout() {}
};

int runMessageLoop(FrameClient& client);

}
