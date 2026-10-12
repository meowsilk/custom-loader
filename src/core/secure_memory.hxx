#pragma once

#include <Windows.h>

#include <cstddef>
#include <new>
#include <string>

namespace loader::core
{

template<typename T>
class SecureAllocator
{
public:
    using value_type = T;

    SecureAllocator() = default;
    template<typename U>
    SecureAllocator(const SecureAllocator<U>&) noexcept
    {
    }

    T* allocate(std::size_t count)
    {
        const std::size_t bytes = count * sizeof(T);
        void* memory = ::operator new(bytes);
        VirtualLock(memory, bytes);
        return static_cast<T*>(memory);
    }

    void deallocate(T* memory, std::size_t count) noexcept
    {
        const std::size_t bytes = count * sizeof(T);
        SecureZeroMemory(memory, bytes);
        VirtualUnlock(memory, bytes);
        ::operator delete(memory);
    }

    template<typename U>
    friend bool operator==(const SecureAllocator&, const SecureAllocator<U>&) noexcept
    {
        return true;
    }
};

using SecureString32 = std::basic_string<char32_t, std::char_traits<char32_t>, SecureAllocator<char32_t>>;

}
