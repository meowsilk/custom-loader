#include "auth/mock_auth_service.hxx"

#include "core/build_config.hxx"

namespace loader::auth
{
namespace
{

constexpr int kMinLatencyMs = 600;
constexpr int kMaxLatencyMs = 1000;

class MockOperation final : public PendingOperation
{
public:
    MockOperation(Scheduler& scheduler, TimerId timer) : scheduler_(scheduler), timer_(timer) {}
    ~MockOperation() override { cancel(); }

    void cancel() override { scheduler_.cancel(timer_); }

private:
    Scheduler& scheduler_;
    TimerId timer_;
};

AuthResult decideSignIn(std::u32string_view username)
{
    if constexpr (core::kDebugBuild)
    {
        if (username == U"error.invalid")
            return AuthResult::failure(AuthError::InvalidCredentials);
        if (username == U"error.network")
            return AuthResult::failure(AuthError::NetworkUnavailable);
        if (username == U"error.locked")
            return AuthResult::failure(AuthError::AccountLocked);
    }
    return AuthResult::ok();
}

AuthResult decideRegistration(std::u32string_view username)
{
    if constexpr (core::kDebugBuild)
    {
        if (username == U"taken.name")
            return AuthResult::failure(AuthError::UsernameTaken);
        if (username == U"error.network")
            return AuthResult::failure(AuthError::NetworkUnavailable);
    }
    return AuthResult::ok();
}

AuthResult decideActivation(std::u32string_view licenseKey)
{
    if constexpr (core::kDebugBuild)
    {
        if (licenseKey.starts_with(U"ZZZZ"))
            return AuthResult::failure(AuthError::InvalidLicense);
        if (licenseKey.starts_with(U"NNNN"))
            return AuthResult::failure(AuthError::NetworkUnavailable);
    }

    if (licenseKey.starts_with(U"DAY1"))
        return AuthResult::activated(LicenseDuration::OneDay);
    if (licenseKey.starts_with(U"WEEK"))
        return AuthResult::activated(LicenseDuration::OneWeek);
    if (licenseKey.starts_with(U"LIFE"))
        return AuthResult::activated(LicenseDuration::Lifetime);

    unsigned sum = 0;
    for (const char32_t character : licenseKey)
        sum += static_cast<unsigned>(character);
    return AuthResult::activated(static_cast<LicenseDuration>(sum % 3));
}

}

MockAuthService::MockAuthService(Scheduler& scheduler)
    : scheduler_(scheduler), random_(std::random_device{}())
{
}

std::chrono::milliseconds MockAuthService::randomLatency()
{
    return std::chrono::milliseconds(
        std::uniform_int_distribution<int>(kMinLatencyMs, kMaxLatencyMs)(random_));
}

std::unique_ptr<PendingOperation> MockAuthService::schedule(AuthResult result, Completion completion)
{
    const TimerId timer = scheduler_.post(
        randomLatency(), [result, completion = std::move(completion)]() mutable { completion(result); });
    return std::make_unique<MockOperation>(scheduler_, timer);
}

std::unique_ptr<PendingOperation> MockAuthService::signIn(SignInRequest request, Completion completion)
{
    return schedule(decideSignIn(request.username), std::move(completion));
}

std::unique_ptr<PendingOperation> MockAuthService::registerAccount(RegisterRequest request,
                                                                   Completion completion)
{
    return schedule(decideRegistration(request.username), std::move(completion));
}

std::unique_ptr<PendingOperation> MockAuthService::activateProduct(ActivationRequest request,
                                                                   Completion completion)
{
    return schedule(decideActivation(request.licenseKey), std::move(completion));
}

}
