#include "platform/message_loop.hxx"

#include <algorithm>

namespace loader::platform
{
namespace
{

constexpr DWORD kFrameWatchdogMs = 250;

bool drainMessages(int& exitCode)
{
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            exitCode = static_cast<int>(message.wParam);
            return false;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return true;
}

DWORD toWaitMilliseconds(const std::optional<std::chrono::milliseconds>& timeout)
{
    if (!timeout)
        return INFINITE;
    return static_cast<DWORD>(std::clamp<long long>(timeout->count(), 1, 60'000));
}

}

int runMessageLoop(FrameClient& client)
{
    int exitCode = 0;

    while (drainMessages(exitCode))
    {
        if (!client.wantsFrame())
        {
            const DWORD result = MsgWaitForMultipleObjectsEx(
                0, nullptr, toWaitMilliseconds(client.idleTimeout()), QS_ALLINPUT, MWMO_INPUTAVAILABLE);
            if (result == WAIT_TIMEOUT)
                client.onIdleTimeout();
            continue;
        }

        HANDLE frameReady = client.frameReadyEvent();
        if (frameReady == nullptr)
        {
            client.renderFrame();
            continue;
        }

        const DWORD result =
            MsgWaitForMultipleObjectsEx(1, &frameReady, kFrameWatchdogMs, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
        if (result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT)
            client.renderFrame();
    }

    return exitCode;
}

}
