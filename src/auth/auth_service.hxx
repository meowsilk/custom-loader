#pragma once

#include "auth/auth_types.hxx"
#include "auth/scheduler.hxx"

#include <functional>
#include <memory>

namespace loader::auth
{

using Completion = std::function<void(AuthResult)>;

class PendingOperation
{
public:
    virtual ~PendingOperation() = default;

    virtual void cancel() = 0;
};

class AuthService
{
public:
    virtual ~AuthService() = default;

    virtual std::unique_ptr<PendingOperation> signIn(SignInRequest request, Completion completion) = 0;
    virtual std::unique_ptr<PendingOperation> registerAccount(RegisterRequest request,
                                                              Completion completion) = 0;
    virtual std::unique_ptr<PendingOperation> activateProduct(ActivationRequest request,
                                                              Completion completion) = 0;
};

}
