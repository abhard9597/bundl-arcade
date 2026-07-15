#include <wx/wx.h>
#include "SnakeGame.h"

class MainFrame final : public wxFrame
{
public:
    MainFrame()
        : wxFrame(nullptr, wxID_ANY, "Venom dash", wxDefaultPosition, wxSize(980, 680))
    {
        auto *panel = CreateGamePanel(this);
        panel->SetFocus();
        panel->SetFocusFromKbd();
        Centre();
    }
};

class SnakeApp final : public wxApp
{
public:
    bool OnInit() override
    {
        if (!wxApp::OnInit())
            return false;
        auto *f = new MainFrame();
        f->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(SnakeApp);
