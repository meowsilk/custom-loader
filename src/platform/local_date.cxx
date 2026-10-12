#include "platform/local_date.hxx"

#include <Windows.h>

namespace loader::platform
{
namespace
{

void appendNumber(std::u32string& text, unsigned value, int digits)
{
    std::u32string number;
    for (int i = 0; i < digits; ++i)
    {
        number.insert(number.begin(), static_cast<char32_t>(U'0' + value % 10));
        value /= 10;
    }
    text += number;
}

}

std::u32string localDateText()
{
    SYSTEMTIME now{};
    GetLocalTime(&now);

    std::u32string text;
    appendNumber(text, now.wDay, 2);
    text += U'.';
    appendNumber(text, now.wMonth, 2);
    text += U'.';
    appendNumber(text, now.wYear, 4);
    return text;
}

}
