#pragma once

#include "ui/widget.hxx"

#include <vector>

namespace loader::ui::input
{

class FocusManager
{
public:
    void setRoot(Widget* root);

    Widget* focused() const { return focused_; }
    void focus(Widget* widget, bool focusVisible);
    void clear();
    void forget(const Widget* widget);
    void advance(bool backwards);

private:
    void collect(Widget& widget, std::vector<Widget*>& order) const;

    Widget* root_ = nullptr;
    Widget* focused_ = nullptr;
};

}
