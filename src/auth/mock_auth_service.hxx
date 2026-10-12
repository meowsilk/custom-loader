#pragma once

#include "auth/auth_service.hxx"

#include <random>

namespace loader::auth
{

class MockAuthService final : public AuthService
{
public:
    explicit MockAuthService(Scheduler& scheduler);

    std::unique_ptr<PendingOperation> signIn(SignInRequest request, Completion completion) override;
    std::unique_ptr<PendingOperation> registerAccount(RegisterRequest request,
                                                      Completion completion) override;
    std::unique_ptr<PendingOperation> activateProduct(ActivationRequest request,
                                                      Completion completion) override;

private:
    std::unique_ptr<PendingOperation> schedule(AuthResult result, Completion completion);
    std::chrono::milliseconds randomLatency();

    Scheduler& scheduler_;
    std::mt19937 random_;
};

}
