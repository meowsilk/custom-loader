#pragma once

namespace loader::core::input
{

enum class PointerButton
{
    Left,
    Right,
    Middle,
};

enum class Cursor
{
    Arrow,
    IBeam,
    Hand,
};

enum class Key
{
    Unknown,
    Tab,
    Enter,
    Escape,
    Space,
    Backspace,
    Delete,
    Left,
    Right,
    Up,
    Down,
    Home,
    End,
    Insert,
    A,
    C,
    V,
    X,
    Y,
    Z,
    F9,
    F10,
};

struct Modifiers
{
    bool shift = false;
    bool control = false;
    bool alt = false;
};

struct KeyEvent
{
    Key key = Key::Unknown;
    Modifiers modifiers;
    bool repeat = false;
};

}
