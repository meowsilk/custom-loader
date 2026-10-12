#include "platform/keyboard.hxx"

namespace loader::platform
{

core::input::Key keyFromVirtualKey(WPARAM virtualKey)
{
    using core::input::Key;

    switch (virtualKey)
    {
    case VK_TAB:
        return Key::Tab;
    case VK_RETURN:
        return Key::Enter;
    case VK_ESCAPE:
        return Key::Escape;
    case VK_SPACE:
        return Key::Space;
    case VK_BACK:
        return Key::Backspace;
    case VK_DELETE:
        return Key::Delete;
    case VK_LEFT:
        return Key::Left;
    case VK_RIGHT:
        return Key::Right;
    case VK_UP:
        return Key::Up;
    case VK_DOWN:
        return Key::Down;
    case VK_HOME:
        return Key::Home;
    case VK_END:
        return Key::End;
    case VK_INSERT:
        return Key::Insert;
    case 'A':
        return Key::A;
    case 'C':
        return Key::C;
    case 'V':
        return Key::V;
    case 'X':
        return Key::X;
    case 'Y':
        return Key::Y;
    case 'Z':
        return Key::Z;
    case VK_F9:
        return Key::F9;
    case VK_F10:
        return Key::F10;
    default:
        return Key::Unknown;
    }
}

core::input::Modifiers currentModifiers()
{
    const auto pressed = [](int key) { return (GetKeyState(key) & 0x8000) != 0; };
    return {pressed(VK_SHIFT), pressed(VK_CONTROL), pressed(VK_MENU)};
}

}
