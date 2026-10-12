#pragma once

#include "storage/atomic_file.hxx"

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace loader::storage
{

std::optional<std::vector<std::byte>> protectForCurrentUser(std::span<const std::byte> plaintext);
std::optional<SecureBytes> unprotectForCurrentUser(std::span<const std::byte> ciphertext);

}
