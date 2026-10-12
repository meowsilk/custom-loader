#pragma once

#include "auth/auth_types.hxx"
#include "auth/validation.hxx"
#include "ui/localization.hxx"

namespace loader::screens
{

ui::Str describeUsernameIssue(auth::FieldIssue issue);
ui::Str describePasswordIssue(auth::FieldIssue issue);
ui::Str describeConfirmationIssue(auth::FieldIssue issue);
ui::Str describeLicenseIssue(auth::FieldIssue issue);
ui::Str describeAuthError(auth::AuthError error);

}
