#include "PauseDialog.h"

#include <wx/dcbuffer.h>
#include <wx/statline.h>
#include <wx/scrolwin.h>
#include <wx/app.h>
#include <wx/simplebook.h>

#include "App.h"
#include "SoundManager.h"
#include "AppTheme.h"

namespace
{
    const AppTheme::Palette THEME = AppTheme::GetPalette();

    const wxColour BG = THEME.bg;
    const wxColour PANEL = THEME.card;
    const wxColour TEXT = THEME.text;
    const wxColour SUBTLE = THEME.subtle;
    const wxColour LINE = THEME.cardEdge;

    const wxColour BTN = THEME.button;
    const wxColour BTN_HOVER = THEME.buttonHover;
    const wxColour BTN_PRESS = THEME.buttonPress;

    class FlatButton : public wxPanel
    {
    public:
        FlatButton(wxWindow *parent, const wxString &label)
            : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
              m_label(label), m_fill(BTN)
        {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetBackgroundColour(parent->GetBackgroundColour());
            SetCursor(wxCursor(wxCURSOR_HAND));
            SetMinSize(wxSize(150, 38));

            Bind(wxEVT_PAINT, &FlatButton::OnPaint, this);
            Bind(wxEVT_ENTER_WINDOW, &FlatButton::OnEnter, this);
            Bind(wxEVT_LEAVE_WINDOW, &FlatButton::OnLeave, this);
            Bind(wxEVT_LEFT_DOWN, &FlatButton::OnDown, this);
            Bind(wxEVT_LEFT_UP, &FlatButton::OnUp, this);
        }

        void SetLabelText(const wxString &s)
        {
            m_label = s;
            Refresh();
        }
        void SetOnClick(std::function<void()> fn) { m_onClick = std::move(fn); }

    private:
        void OnEnter(wxMouseEvent &e)
        {
            m_hover = true;
            if (!m_pressed)
                m_fill = BTN_HOVER;
            Refresh();
            e.Skip();
        }
        void OnLeave(wxMouseEvent &e)
        {
            m_hover = false;
            if (!m_pressed)
                m_fill = BTN;
            Refresh();
            e.Skip();
        }
        void OnDown(wxMouseEvent &e)
        {
            m_pressed = true;
            m_fill = BTN_PRESS;
            Refresh();
            if (!HasCapture())
                CaptureMouse();
            e.Skip();
        }

        void OnUp(wxMouseEvent &e)
        {
            if (HasCapture())
                ReleaseMouse();
            bool wasPressed = m_pressed;
            m_pressed = false;

            wxPoint p = ScreenToClient(wxGetMousePosition());
            bool inside = GetClientRect().Contains(p);

            m_fill = (m_hover ? BTN_HOVER : BTN);
            Refresh();

            if (wasPressed && inside && m_onClick)
            {
                wxGetApp().GetSfx().PlayClick();
                auto fn = m_onClick;
                wxTheApp->CallAfter([fn]()
                                    { fn(); });
            }
            e.Skip();
        }

        void OnPaint(wxPaintEvent &)
        {
            wxAutoBufferedPaintDC dc(this);

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(GetBackgroundColour()));
            wxSize sz = GetClientSize();
            dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

            int w = sz.GetWidth();
            int h = sz.GetHeight();
            const int radius = 14;

            dc.SetBrush(wxBrush(wxColour(0, 0, 0, 35)));
            dc.DrawRoundedRectangle(3, 4, w - 4, h - 4, radius);

            dc.SetBrush(wxBrush(m_fill));
            dc.DrawRoundedRectangle(0, 0, w - 6, h - 6, radius);

            dc.SetBrush(wxBrush(wxColour(160, 205, 235)));
            dc.DrawRoundedRectangle(0, 0, w - 6, 4, radius);

            dc.SetTextForeground(TEXT);
            dc.SetFont(wxFontInfo(13).Bold().FaceName("Avenir Next"));
            wxCoord tw, th;
            dc.GetTextExtent(m_label, &tw, &th);
            dc.DrawText(m_label, (w - 6 - tw) / 2, (h - 6 - th) / 2);
        }

        wxString m_label;
        std::function<void()> m_onClick;
        wxColour m_fill;
        bool m_hover = false, m_pressed = false;
    };

    wxPanel *MakeCard(wxWindow *parent, const wxString &title, wxStaticText *&outBody)
    {
        auto *card = new wxPanel(parent);
        card->SetBackgroundColour(PANEL);

        auto *root = new wxBoxSizer(wxVERTICAL);

        auto *t = new wxStaticText(card, wxID_ANY, title);
        t->SetForegroundColour(TEXT);
        t->SetFont(wxFontInfo(22).Bold().FaceName("Avenir Next"));

        auto *divider = new wxStaticLine(card);
        divider->SetForegroundColour(LINE);

        outBody = new wxStaticText(card, wxID_ANY, "");
        outBody->SetForegroundColour(SUBTLE);
        outBody->SetFont(wxFontInfo(15).FaceName("Avenir Next"));

        card->Bind(wxEVT_SIZE, [outBody, card](wxSizeEvent &e)
                   {
            int w = card->GetClientSize().GetWidth();
            outBody->Wrap(std::max(200, w - 40));
            e.Skip(); });

        root->Add(t, 0, wxALL, 18);
        root->Add(divider, 0, wxEXPAND | wxLEFT | wxRIGHT, 18);
        root->Add(outBody, 0, wxALL, 18);

        card->SetSizer(root);
        return card;
    }

    static wxString FormatStats(const GameStats &s)
    {
        return wxString::Format(
            "Games Played: %d\n"
            "Best Score: %d\n"
            "Total Score: %d\n"
            "Total Words Found: %d\n"
            "Total Time Played: %d seconds\n\n"
            "Average Score: %.2f\n"
            "Average Words/Round: %.2f\n"
            "Average Time/Round: %.2f seconds\n",
            s.gamesPlayed,
            s.bestScore,
            s.totalScore,
            s.totalWordsFound,
            s.totalSecondsPlayed,
            s.AvgScore(),
            s.AvgWords(),
            s.AvgSeconds());
    }

    wxWindow *MakeTutorialPage(wxWindow *parent)
    {
        auto *scroller = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
        scroller->SetBackgroundColour(BG);
        scroller->SetScrollRate(10, 10);

        auto *root = new wxBoxSizer(wxVERTICAL);

        wxStaticText *body = nullptr;
        auto *card = MakeCard(scroller, "Tutorial", body);

        body->SetLabel(
            "Goal:\n"
            "Make as many real words as you can before the timer ends.\n\n"
            "Shortcuts:\n"
            "• Press S to shuffle the rack.\n"
            "• Press Esc to pause during a round.\n\n"
            "Scoring:\n"
            "• Longer words score more points (length²).\n");

        root->Add(card, 0, wxEXPAND | wxALL, 20);
        root->AddStretchSpacer(1);

        scroller->SetSizer(root);
        scroller->FitInside();
        return scroller;
    }

    wxWindow *MakeStatsPage(wxWindow *parent,
                            const GameStats &statsSnapshot,
                            std::function<GameStats()> getStats,
                            std::function<void()> onResetStats,
                            wxStaticText *&outStatsLabel)
    {
        auto *scroller = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
        scroller->SetBackgroundColour(BG);
        scroller->SetScrollRate(10, 10);

        auto *root = new wxBoxSizer(wxVERTICAL);

        wxStaticText *body = nullptr;
        auto *card = MakeCard(scroller, "Statistics", body);
        body->SetLabel(FormatStats(statsSnapshot));
        outStatsLabel = body;

        root->Add(card, 0, wxEXPAND | wxALL, 20);

        if (onResetStats && getStats)
        {
            auto *row = new wxBoxSizer(wxHORIZONTAL);
            auto *btnReset = new FlatButton(scroller, "Reset Stats");
            row->AddStretchSpacer(1);
            row->Add(btnReset, 0);

            btnReset->SetOnClick([scroller, outStatsLabel, getStats, onResetStats]
                                 {
                onResetStats();                 // reset data
                GameStats fresh = getStats();   // fetch updated stats
                outStatsLabel->SetLabel(FormatStats(fresh));
                outStatsLabel->Wrap(std::max(200, scroller->GetClientSize().GetWidth() - 40));
                scroller->Layout();
                scroller->Refresh(); });

            root->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 20);
        }

        root->AddStretchSpacer(1);

        scroller->SetSizer(root);
        scroller->FitInside();
        return scroller;
    }
}

PauseDialog::PauseDialog(wxWindow *parent,
                         Tab startTab,
                         bool showResume,
                         const GameStats &statsSnapshot,
                         std::function<GameStats()> getStats,
                         std::function<void()> onResetStats)
    : wxDialog(parent, wxID_ANY,
               showResume ? "Paused" : "Info",
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetBackgroundColour(BG);

    if (parent)
    {
        SetSize(parent->GetSize());
        CentreOnParent();
    }
    else
    {
        SetSize(wxSize(900, 700));
        Centre();
    }
    SetMinSize(wxSize(800, 600));

    auto *root = new wxBoxSizer(wxVERTICAL);

    auto *headerRow = new wxBoxSizer(wxHORIZONTAL);

    auto *header = new wxStaticText(this, wxID_ANY, showResume ? "Game Paused" : "Help & Info");
    header->SetForegroundColour(TEXT);
    header->SetFont(wxFontInfo(26).Bold().FaceName("Avenir Next"));

    auto *btnTut = new FlatButton(this, "Tutorial");
    auto *btnStats = new FlatButton(this, "Statistics");
    auto *btnClose = new FlatButton(this, showResume ? "Resume" : "Close");

    bool sfxOn = wxGetApp().GetSfx().IsEnabled();
    auto *btnSfx = new FlatButton(this, sfxOn ? "SFX: ON" : "SFX: OFF");

    headerRow->Add(header, 0, wxLEFT | wxTOP | wxBOTTOM, 16);
    headerRow->AddStretchSpacer(1);
    headerRow->Add(btnSfx, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    headerRow->Add(btnTut, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    headerRow->Add(btnStats, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    headerRow->Add(btnClose, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 14);

    root->Add(headerRow, 0, wxEXPAND);

    m_book = new wxSimplebook(this, wxID_ANY);

    wxWindow *tutorialPage = MakeTutorialPage(m_book);
    wxStaticText *statsLabel = nullptr;
    wxWindow *statsPage = MakeStatsPage(m_book, statsSnapshot, getStats, onResetStats, statsLabel);

    m_book->AddPage(tutorialPage, "Tutorial");
    m_book->AddPage(statsPage, "Statistics");
    m_book->SetSelection(startTab == Tab::Stats ? 1 : 0);
    root->Add(m_book, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);

    SetSizer(root);

    btnTut->SetOnClick([this]
                       { m_book->SetSelection(0); });
    btnStats->SetOnClick([this]
                         { m_book->SetSelection(1); });
    btnClose->SetOnClick([this]
                         { EndModal(wxID_OK); });

    btnSfx->SetOnClick([btnSfx]
                       {
        bool now = !wxGetApp().GetSfx().IsEnabled();
        wxGetApp().GetSfx().SetEnabled(now);
        btnSfx->SetLabelText(now ? "SFX: ON" : "SFX: OFF");
        if (now) wxGetApp().GetSfx().PlayClick(); });
}