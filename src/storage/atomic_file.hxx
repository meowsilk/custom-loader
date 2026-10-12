#pragma once

#include "core/secure_memory.hxx"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace loader::storage
{

using SecureBytes = std::vector<std::byte, core::SecureAllocator<std::byte>>;

void writeFileAtomically(const std::filesystem::path& path, std::span<const std::byte> bytes);
std::optional<std::vector<std::byte>> readFile(const std::filesystem::path& path);
void removeFileIfPresent(const std::filesystem::path& path);

}
