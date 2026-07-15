#include "RoundOverDialog.h"
#include "AppTheme.h"

#include <wx/dcbuffer.h>
#include <wx/listbox.h>
#include <wx/statline.h>
#include <wx/app.h>
#include <wx/textctrl.h>

#include "App.h"
#include "SoundManager.h"

namespace
{
    const AppTheme::Palette THEME = AppTheme::GetPalette();

    const wxColour BG = THEME.bg;
    const wxColour CARD = THEME.card;
    const wxColour CARD_EDGE = THEME.cardEdge;
    const wxColour TEXT = THEME.text;
    const wxColour SUBTLE = THEME.subtle;
    const wxColour INPUT_BG = THEME.inputBg;
    const wxColour INPUT_FG = THEME.inputFg;
    const wxColour BTN = THEME.button;
    const wxColour BTN_HOVER = THEME.buttonHover;
    const wxColour BTN_PRESS = THEME.buttonPress;

    class RoundedCard : public wxPanel
    {
    public:
        // A simple rounded card panel used for the main content area of the round over dialog
        RoundedCard(wxWindow *parent)
            : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
        {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetBackgroundColour(CARD);
            Bind(wxEVT_PAINT, &RoundedCard::OnPaint, this);
        }

    private:
        // Paints the rounded card with a shadow and border, using the colors defined in the app theme
        void OnPaint(wxPaintEvent &)
        {
            wxAutoBufferedPaintDC dc(this);

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(GetParent()->GetBackgroundColour()));
            wxSize sz = GetClientSize();
            dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

            int w = sz.GetWidth();
            int h = sz.GetHeight();
            const int radius = 18;

            dc.SetBrush(wxBrush(wxColour(0, 0, 0, 35)));
            dc.DrawRoundedRectangle(6, 7, w - 6, h - 6, radius);

            dc.SetBrush(wxBrush(GetBackgroundColour()));
            dc.DrawRoundedRectangle(0, 0, w - 10, h - 10, radius);

            dc.SetPen(wxPen(CARD_EDGE, 1));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRoundedRectangle(0, 0, w - 10, h - 10, radius);
        }
    };

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
            SetMinSize(wxSize(170, 44));

            Bind(wxEVT_PAINT, &FlatButton::OnPaint, this);
            Bind(wxEVT_ENTER_WINDOW, &FlatButton::OnEnter, this);
            Bind(wxEVT_LEAVE_WINDOW, &FlatButton::OnLeave, this);
            Bind(wxEVT_LEFT_DOWN, &FlatButton::OnDown, this);
            Bind(wxEVT_LEFT_UP, &FlatButton::OnUp, this);
        }

        void SetOnClick(std::function<void()> fn) { m_onClick = std::move(fn); }

    private:
        // Event handlers for hover and click effects, and to trigger the click callback
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
            // Paints the flat button with hover and press effects, using the colors defined in the app theme
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

            // pale blue accent
            dc.SetBrush(wxBrush(wxColour(160, 205, 235)));
            dc.DrawRoundedRectangle(0, 0, w - 6, 4, radius);

            dc.SetTextForeground(TEXT);
            dc.SetFont(wxFontInfo(13).Bold().FaceName("Avenir Next"));
            wxCoord tw, th;
            dc.GetTextExtent(m_label, &tw, &th);
            dc.DrawText(m_label, (w - 6 - tw) / 2, (h - 6 - th) / 2);
        }

    private:
        wxString m_label;
        std::function<void()> m_onClick;
        wxColour m_fill;
        bool m_hover = false, m_pressed = false;
    };
}

RoundOverDialog::RoundOverDialog(wxWindow *parent, int finalScore, const std::vector<std::string> &missedWords)
    : wxDialog(parent, wxID_ANY, "Round Over", wxDefaultPosition, wxSize(860, 620),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetBackgroundColour(BG);
    SetMinSize(wxSize(760, 560));
    CentreOnParent();

    auto *root = new wxBoxSizer(wxVERTICAL);

    auto *title = new wxStaticText(this, wxID_ANY, "Time's Up!");
    title->SetForegroundColour(TEXT);
    title->SetFont(wxFontInfo(28).Bold().FaceName("Avenir Next"));
    root->Add(title, 0, wxLEFT | wxRIGHT | wxTOP, 18);

    auto *score = new wxStaticText(this, wxID_ANY, wxString::Format("Final Score: %d", finalScore));
    score->SetForegroundColour(SUBTLE);
    score->SetFont(wxFontInfo(16).Bold().FaceName("Avenir Next"));
    root->Add(score, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 8);

    auto *card = new RoundedCard(this);
    auto *cs = new wxBoxSizer(wxVERTICAL);
    card->SetSizer(cs);

    auto *label = new wxStaticText(card, wxID_ANY, wxString::Format("Missed Words (%zu)", missedWords.size()));
    label->SetForegroundColour(TEXT);
    label->SetFont(wxFontInfo(16).Bold().FaceName("Avenir Next"));

    auto *line = new wxStaticLine(card);
    line->SetForegroundColour(CARD_EDGE);

    auto *list = new wxTextCtrl(
        card,
        wxID_ANY,
        "",
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
    list->SetOwnBackgroundColour(INPUT_BG);
    list->SetOwnForegroundColour(INPUT_FG);
    list->SetBackgroundColour(INPUT_BG);
    list->SetForegroundColour(INPUT_FG);
    list->SetFont(wxFontInfo(12).FaceName("Avenir Next"));

    for (const auto &w : missedWords)
    {
        if (!list->GetValue().IsEmpty())
            list->AppendText("\n");
        list->AppendText(w);
    }

    list->Refresh();
    list->Update();

    cs->Add(label, 0, wxLEFT | wxRIGHT | wxTOP, 18);
    cs->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 18);
    cs->Add(list, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 18);

    root->Add(card, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 18);

    auto *row = new wxBoxSizer(wxHORIZONTAL);

    auto *btnMenu = new FlatButton(this, "Main Menu");
    auto *btnAgain = new FlatButton(this, "New Game");

    btnMenu->SetOnClick([this]
                        { EndModal(RoundOverDialog::GoMenu); });
    btnAgain->SetOnClick([this]
                         { EndModal(RoundOverDialog::PlayAgain); });

    row->Add(btnMenu, 0, wxRIGHT, 12);
    row->AddStretchSpacer(1);
    row->Add(btnAgain, 0);

    root->Add(row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 18);

    SetSizer(root);
}