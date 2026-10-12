#pragma once

#include "core/geometry.hxx"
#include "ui/animation/animated.hxx"
#include "ui/animation/spring.hxx"
#include "ui/widget.hxx"
#include "ui/widgets/label.hxx"
#include "ui/widgets/language_selector.hxx"
#include "ui/widgets/overlay_stack.hxx"
#include "ui/widgets/panel_surface.hxx"

#include <memory>
#include <vector>

namespace loader::screens
{

class WindowControls
{
public:
    virtual ~WindowControls() = default;

    virtual void minimizeWindow() = 0;
    virtual void closeWindow() = 0;
};

enum class TransitionKind
{
    SlideForward,
    SlideBackward,
    Morph,
};

class Shell : public ui::Widget
{
public:
    static constexpr float kCustomHeaderHeight = 52.0F;

    Shell(ui::UiContext& context, WindowControls& controls);

    void setBackdropAvailable(bool available);
    void setReducedMotion(bool reduced) { reducedMotion_ = reduced; }
    void setMaxPanelSize(core::Size size);

    void showInitial(std::unique_ptr<ui::Widget> content,
                     core::Size panelSize,
                     std::unique_ptr<ui::Widget> header = nullptr);
    void transitionTo(std::unique_ptr<ui::Widget> content,
                      core::Size panelSize,
                      TransitionKind kind,
                      std::unique_ptr<ui::Widget> header = nullptr);

    core::Rect panelRect() const { return panel_->bounds(); }
    core::Rect headerRect() const { return header_->bounds(); }
    bool transitioning() const { return transition_.active; }

protected:
    void layoutChildren(core::Rect bounds) override;
    bool updateSelf(float deltaSeconds) override;

private:
    struct Transition
    {
        bool active = false;
        TransitionKind kind = TransitionKind::SlideForward;
        float elapsed = 0.0F;
        float duration = 0.0F;
        ui::Widget* outgoing = nullptr;
        ui::Widget* incoming = nullptr;
        std::unique_ptr<ui::Widget> header;
        bool headerChange = false;
        bool headerApplied = false;
    };

    void applyHeader(std::unique_ptr<ui::Widget> header);
    core::Size hostSizeFor(core::Size panel, bool customHeader) const;
    void updateTransition(float deltaSeconds);
    void applyTransitionFrame();
    void finishTransition();
    void resizePanel(core::Size size, bool animate, bool morph);

    ui::PanelSurface* panel_ = nullptr;
    ui::Widget* header_ = nullptr;
    ui::Widget* headerSlot_ = nullptr;
    ui::Widget* headerContent_ = nullptr;
    ui::Label* title_ = nullptr;
    ui::LanguageSelector* languageSelector_ = nullptr;
    ui::Widget* minimize_ = nullptr;
    ui::Widget* close_ = nullptr;
    std::vector<std::unique_ptr<ui::Widget>> retired_;
    ui::OverlayStack* contentHost_ = nullptr;
    ui::animation::Spring width_;
    ui::animation::Spring height_;
    Transition transition_;
    core::Size maxPanel_{100000.0F, 100000.0F};
    core::Size requestedPanel_;
    bool reducedMotion_ = false;
    bool sized_ = false;
};

}
