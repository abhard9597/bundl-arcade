#include "GamePanel.h"
#include "App.h"
#include "SoundManager.h"
#include "AppTheme.h"

#include <wx/dcbuffer.h>
#include <wx/app.h>

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

    const wxColour BTN_ACCENT = AppTheme::IsDarkMode()
                                    ? wxColour(80, 175, 170)
                                    : wxColour(91, 155, 213);

    const wxColour SHADOW = THEME.shadow;

    // Rounded card
    class RoundedCard : public wxPanel
    {
    public:
        RoundedCard(wxWindow *parent)
            : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
        {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetBackgroundColour(CARD);
            Bind(wxEVT_PAINT, &RoundedCard::OnPaint, this);
        }

    private:
        void OnPaint(wxPaintEvent &)
        {
            wxAutoBufferedPaintDC dc(this);
            dc.Clear();

            wxSize sz = GetClientSize();
            int w = sz.GetWidth();
            int h = sz.GetHeight();
            const int radius = 18;

            dc.SetPen(*wxTRANSPARENT_PEN);

            // shadow
            dc.SetBrush(wxBrush(SHADOW));
            dc.DrawRoundedRectangle(6, 7, w - 6, h - 6, radius);

            // card
            dc.SetBrush(wxBrush(GetBackgroundColour()));
            dc.DrawRoundedRectangle(0, 0, w - 10, h - 10, radius);

            // border highlight
            dc.SetPen(wxPen(CARD_EDGE, 1));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRoundedRectangle(0, 0, w - 10, h - 10, radius);
        }
    };

}

class FlatButton : public wxPanel
{
public:
    FlatButton(wxWindow *parent, const wxString &label, bool playClick = true)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
          m_label(label),
          m_fill(BTN),
          m_playClick(playClick)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        SetBackgroundColour(parent->GetBackgroundColour());
        SetCursor(wxCursor(wxCURSOR_HAND));
        SetMinSize(wxSize(130, 36));

        // Bind events
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
            if (m_playClick)
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

        // Fill panel with parent background so corners blend
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(GetBackgroundColour()));
        wxSize sz = GetClientSize();
        dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

        int w = sz.GetWidth();
        int h = sz.GetHeight();
        const int radius = 14;

        // soft shadow
        dc.SetBrush(wxBrush(wxColour(0, 0, 0, 35)));
        dc.DrawRoundedRectangle(3, 4, w - 4, h - 4, radius);

        // button body
        dc.SetBrush(wxBrush(m_fill));
        dc.DrawRoundedRectangle(0, 0, w - 6, h - 6, radius);

        // pale blue accent line
        dc.SetBrush(wxBrush(wxColour(160, 205, 235)));
        dc.DrawRoundedRectangle(0, 0, w - 6, 4, radius);

        // text
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
    bool m_hover = false;
    bool m_pressed = false;
    bool m_playClick = true;
};

GamePanel::GamePanel(wxWindow *parent) : wxPanel(parent)
{
    SetBackgroundColour(BG);
    SetForegroundColour(TEXT);

    auto *root = new wxBoxSizer(wxVERTICAL);

    // Top bar
    auto *top = new wxBoxSizer(wxHORIZONTAL);

    m_timeLabel = new wxStaticText(this, wxID_ANY, "Time: 01:00");
    m_timeLabel->SetForegroundColour(TEXT);
    m_timeLabel->SetFont(wxFontInfo(14).Bold().FaceName("Avenir Next"));

    m_scoreLabel = new wxStaticText(this, wxID_ANY, "Score: 0");
    m_scoreLabel->SetForegroundColour(TEXT);
    m_scoreLabel->SetFont(wxFontInfo(14).Bold().FaceName("Avenir Next"));

    m_backBtn = new FlatButton(this, "Main Menu");
    m_pauseBtn = new FlatButton(this, "Pause");

    top->Add(m_timeLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 12);
    top->AddSpacer(14);
    top->Add(m_scoreLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 12);
    top->AddStretchSpacer(1);
    top->Add(m_backBtn, 0, wxALL, 10);
    top->Add(m_pauseBtn, 0, wxALL, 10);

    root->Add(top, 0, wxEXPAND);

    // Main content
    auto *content = new wxBoxSizer(wxHORIZONTAL);

    // Left card
    auto *leftCard = new RoundedCard(this);
    auto *left = new wxBoxSizer(wxVERTICAL);

    auto *rackRow = new wxBoxSizer(wxHORIZONTAL);

    m_rackLabel = new wxStaticText(leftCard, wxID_ANY, "A E T R N S");
    m_rackLabel->SetForegroundColour(TEXT);
    m_rackLabel->SetFont(wxFontInfo(30).Bold().FaceName("Avenir Next"));

    m_shuffleBtn = new FlatButton(leftCard, "Shuffle");
    m_shuffleBtn->SetMinSize(wxSize(130, 36));

    rackRow->Add(m_rackLabel, 0, wxALIGN_CENTER_VERTICAL);
    rackRow->AddStretchSpacer(1);
    rackRow->Add(m_shuffleBtn, 0, wxALIGN_CENTER_VERTICAL);

    m_statusLabel = new wxStaticText(leftCard, wxID_ANY, "Make as many words as you can!");
    m_statusLabel->SetForegroundColour(SUBTLE);
    m_statusLabel->SetFont(wxFontInfo(13).FaceName("Avenir Next"));

    auto *hint = new wxStaticText(leftCard, wxID_ANY, "Type a word and press Enter");
    hint->SetForegroundColour(SUBTLE);
    hint->SetFont(wxFontInfo(12).FaceName("Avenir Next"));

    left->AddSpacer(8);
    left->Add(rackRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 18);
    left->Add(m_statusLabel, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    left->Add(hint, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    left->AddSpacer(12);

    auto *entryRow = new wxBoxSizer(wxHORIZONTAL);

    long guessStyle = wxTE_PROCESS_ENTER;
#ifdef __WXMSW__
    guessStyle |= wxTE_RICH2;
#endif

    m_guessInput = new wxTextCtrl(leftCard, wxID_ANY, "", wxDefaultPosition, wxSize(260, -1),
                                  guessStyle);
    m_guessInput->SetHint("Enter a word...");
    m_guessInput->SetOwnBackgroundColour(INPUT_BG);
    m_guessInput->SetOwnForegroundColour(INPUT_FG);
    m_guessInput->SetBackgroundColour(INPUT_BG);
    m_guessInput->SetForegroundColour(INPUT_FG);
    m_guessInput->SetDefaultStyle(wxTextAttr(INPUT_FG, INPUT_BG));
    m_guessInput->SetFont(wxFontInfo(13).FaceName("Avenir Next"));
    m_guessInput->Refresh();
    m_guessInput->Update();

    m_submitBtn = new FlatButton(leftCard, "Submit", false);
    m_submitBtn->SetMinSize(wxSize(130, 36));

    entryRow->Add(m_guessInput, 1, wxRIGHT, 10);
    entryRow->Add(m_submitBtn, 0);

    left->Add(entryRow, 0, wxEXPAND | wxLEFT | wxRIGHT, 18);
    left->AddSpacer(16);

    leftCard->SetSizer(left);
    content->Add(leftCard, 1, wxEXPAND | wxALL, 14);

    // Right card
    auto *rightCard = new RoundedCard(this);
    rightCard->SetBackgroundColour(CARD);
    rightCard->SetForegroundColour(TEXT);

    auto *right = new wxBoxSizer(wxVERTICAL);

    auto *foundTitle = new wxStaticText(rightCard, wxID_ANY, "Found Words");
    foundTitle->SetForegroundColour(TEXT);
    foundTitle->SetFont(wxFontInfo(16).Bold().FaceName("Avenir Next"));

    m_foundList = new wxTextCtrl(
        rightCard,
        wxID_ANY,
        "",
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
    m_foundList->SetOwnBackgroundColour(INPUT_BG);
    m_foundList->SetOwnForegroundColour(INPUT_FG);
    m_foundList->SetBackgroundColour(INPUT_BG);
    m_foundList->SetForegroundColour(INPUT_FG);
    m_foundList->SetFont(wxFontInfo(12).FaceName("Avenir Next"));
    m_foundList->Refresh();
    m_foundList->Update();

    right->AddSpacer(10);
    right->Add(foundTitle, 0, wxLEFT | wxRIGHT | wxTOP, 18);
    right->AddSpacer(10);
    right->Add(m_foundList, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 18);

    rightCard->SetSizer(right);
    rightCard->SetMinSize(wxSize(340, -1));
    content->Add(rightCard, 0, wxEXPAND | wxALL, 14);

    root->Add(content, 1, wxEXPAND);
    SetSizer(root);

    // Wiring callbacks (deferred -> safe)
    m_backBtn->SetOnClick([this]
                          { if (m_onBack) m_onBack(); });
    m_pauseBtn->SetOnClick([this]
                           { if (m_onPause) m_onPause(); });
    m_shuffleBtn->SetOnClick([this]
                             { if (m_onShuffle) m_onShuffle(); });
    m_submitBtn->SetOnClick([this]
                            { Submit(); });

    m_guessInput->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent &)
                       { Submit(); });

    m_guessInput->Bind(wxEVT_TEXT, [this](wxCommandEvent &e)
                       {
                           m_guessInput->SetDefaultStyle(wxTextAttr(INPUT_FG, INPUT_BG));
                           e.Skip();
                       });
}

void GamePanel::SetTime(int secondsLeft)
{
    int mm = secondsLeft / 60;
    int ss = secondsLeft % 60;
    m_timeLabel->SetLabel(wxString::Format("Time: %02d:%02d", mm, ss));
}

void GamePanel::SetScore(int score)
{
    m_scoreLabel->SetLabel(wxString::Format("Score: %d", score));
}

void GamePanel::SetRack(const wxString &rackSpacedUpper)
{
    m_rackLabel->SetLabel(rackSpacedUpper);
}

void GamePanel::SetStatus(const wxString &status)
{
    m_statusLabel->SetLabel(status);
}

void GamePanel::AddFoundWord(const wxString &word)
{
    if (!m_foundList)
        return;

    if (!m_foundList->GetValue().IsEmpty())
        m_foundList->AppendText("\n");

    m_foundList->AppendText(word);
}

void GamePanel::ResetRoundUI()
{
    m_foundList->Clear();
    m_guessInput->Clear();
    m_guessInput->SetFocus();
}

void GamePanel::EnableGameplay(bool enabled)
{
    m_guessInput->Enable(enabled);
    if (m_backBtn)
        m_backBtn->Enable(enabled);
    if (m_pauseBtn)
        m_pauseBtn->Enable(enabled);
    if (m_shuffleBtn)
        m_shuffleBtn->Enable(enabled);
    if (m_submitBtn)
        m_submitBtn->Enable(enabled);
}

void GamePanel::Submit()
{
    wxString raw = m_guessInput->GetValue();
    raw.Trim(true).Trim(false);
    if (raw.IsEmpty())
        return;

    std::string word(raw.mb_str());
    if (m_onSubmit)
        m_onSubmit(word);

    m_guessInput->Clear();
    m_guessInput->SetFocus();
}
