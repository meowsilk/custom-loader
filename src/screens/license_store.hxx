#pragma once

#include "auth/auth_types.hxx"

#include <cstddef>
#include <map>
#include <optional>

namespace loader::screens
{

class LicenseStore
{
public:
    std::optional<auth::LicenseDuration> find(std::size_t product) const
    {
        const auto entry = entries_.find(product);
        if (entry == entries_.end())
            return std::nullopt;
        return entry->second;
    }

    void set(std::size_t product, auth::LicenseDuration duration) { entries_[product] = duration; }

private:
    std::map<std::size_t, auth::LicenseDuration> entries_;
};

}
