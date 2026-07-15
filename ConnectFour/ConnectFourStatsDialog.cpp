/**
 * @file ConnectFourStatsDialog.cpp
 * @brief Implements the Connect Four statistics dialog.
 *
 * This file builds the themed statistics window, formats each mode section,
 * and wires the reset button so the menu can reload the current values after a
 * reset.
 *
 */

#include "ConnectFourStatsDialog.h"

#include <algorithm>

#include <wx/statline.h>
#include <wx/scrolwin.h>

#include "ConnectFourAppTheme.h"

namespace
{
    const AppTheme::Palette THEME = AppTheme::GetPalette();

    const wxColour BG = THEME.frameBg;
    const wxColour PANEL = THEME.card;
    const wxColour TEXT = THEME.text;
    const wxColour SUBTLE = THEME.subtleText;
    const wxColour LINE = THEME.line;

    /**
     * @brief Converts one mode section into multi-line display text.
     *
     * The helper centralizes the formatting so every mode section is shown in a
     * consistent way inside the dialog cards.
     *
     * @param stats Statistics section to format.
     * @param winsLabel Label used for the wins row.
     * @param lossesLabel Label used for the losses row.
     * @return wxString Formatted multi-line statistics text.
     */
    wxString FormatModeStats(const ConnectFourModeStats& stats,
                             const wxString& winsLabel = "Wins",
                             const wxString& lossesLabel = "Losses")
    {
        return wxString::Format(
            "Games Played: %d\n"
            "%s: %d\n"
            "%s: %d\n"
            "Draws: %d",
            stats.gamesPlayed,
            winsLabel,
            stats.wins,
            lossesLabel,
            stats.losses,
            stats.draws);
    }

    /**
     * @brief Creates one themed card used inside the statistics dialog.
     *
     * Each card contains a heading, a divider line, and a body label whose text
     * wraps automatically when the card is resized.
     *
     * @param parent Parent window that will own the card.
     * @param title Heading text shown at the top of the card.
     * @param body Reference to the body-label pointer that will be assigned.
     * @return wxPanel* Pointer to the created card panel.
     */
    wxPanel* MakeCard(wxWindow* parent, const wxString& title, wxStaticText*& body)
    {
        auto* card = new wxPanel(parent);
        card->SetBackgroundColour(PANEL);

        auto* root = new wxBoxSizer(wxVERTICAL);

        auto* heading = new wxStaticText(card, wxID_ANY, title);
        heading->SetForegroundColour(TEXT);
        heading->SetFont(wxFontInfo(22).Bold().FaceName("Avenir Next"));

        auto* divider = new wxStaticLine(card);
        divider->SetForegroundColour(LINE);

        body = new wxStaticText(card, wxID_ANY, "");
        body->SetForegroundColour(SUBTLE);
        body->SetFont(wxFontInfo(15).FaceName("Avenir Next"));

        card->Bind(wxEVT_SIZE, [body, card](wxSizeEvent& event)
        {
            int width = card->GetClientSize().GetWidth();
            body->Wrap(std::max(220, width - 40)); // Keep text readable on smaller dialog sizes.
            event.Skip();
        });

        root->Add(heading, 0, wxALL, 18);
        root->Add(divider, 0, wxEXPAND | wxLEFT | wxRIGHT, 18);
        root->Add(body, 0, wxALL, 18);

        card->SetSizer(root);
        return card;
    }
}

/**
 * @brief Builds the statistics dialog and populates its controls.
 *
 * The dialog shows one card for each tracked game mode, adds a short note that
 * explains the multiplayer interpretation, and binds the reset button so the
 * labels refresh immediately after a reset.
 *
 * @param parent Parent window that owns the dialog.
 * @param statsSnapshot Statistics used to initialize the visible labels.
 * @param getStats Callback used to fetch the newest values after a reset.
 * @param onResetStats Callback used to clear the stored statistics.
 */
ConnectFourStatsDialog::ConnectFourStatsDialog(wxWindow* parent,
                                               const ConnectFourStats& statsSnapshot,
                                               std::function<ConnectFourStats()> getStats,
                                               std::function<void()> onResetStats)
    : wxDialog(parent, wxID_ANY, "Connect Four Statistics",
               wxDefaultPosition, wxSize(820, 650),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetBackgroundColour(BG);
    SetMinSize(wxSize(760, 560));

    auto* root = new wxBoxSizer(wxVERTICAL);

    auto* header = new wxStaticText(this, wxID_ANY, "Connect Four Statistics");
    header->SetForegroundColour(TEXT);
    header->SetFont(wxFontInfo(26).Bold().FaceName("Avenir Next"));
    root->Add(header, 0, wxLEFT | wxTOP | wxRIGHT, 18);

    auto* subheader = new wxStaticText(this, wxID_ANY,
        "Stats are stored locally on this device.");
    subheader->SetForegroundColour(SUBTLE);
    subheader->SetFont(wxFontInfo(12).FaceName("Avenir Next"));
    root->Add(subheader, 0, wxLEFT | wxRIGHT | wxBOTTOM, 18);

    auto* scroller = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scroller->SetBackgroundColour(BG);
    scroller->SetScrollRate(10, 10);

    auto* scrollSizer = new wxBoxSizer(wxVERTICAL);

    scrollSizer->Add(MakeCard(scroller, "Normal Single Player", normalStatsLabel), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);
    scrollSizer->Add(MakeCard(scroller, "Hard Single Player", hardStatsLabel), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);
    scrollSizer->Add(MakeCard(scroller, "Multiplayer", multiplayerStatsLabel), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    auto* note = new wxStaticText(scroller, wxID_ANY,
        "For multiplayer, Wins = Player 1 wins and Losses = Player 2 wins.");
    note->SetForegroundColour(SUBTLE);
    note->SetFont(wxFontInfo(11).FaceName("Avenir Next"));
    scrollSizer->Add(note, 0, wxLEFT | wxRIGHT | wxBOTTOM, 18);

    scroller->SetSizer(scrollSizer);
    scroller->FitInside();

    root->Add(scroller, 1, wxEXPAND | wxLEFT | wxRIGHT, 12);

    auto* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    auto* resetButton = new wxButton(this, wxID_ANY, "Reset Stats");
    auto* closeButton = new wxButton(this, wxID_OK, "Close");

    buttonRow->AddStretchSpacer(1);
    buttonRow->Add(resetButton, 0, wxRIGHT, 10);
    buttonRow->Add(closeButton, 0);

    root->Add(buttonRow, 0, wxEXPAND | wxALL, 12);

    SetSizer(root);

    RefreshStatsLabels(statsSnapshot);

    resetButton->Bind(wxEVT_BUTTON, [this, getStats, onResetStats](wxCommandEvent&)
    {
        if (!onResetStats || !getStats) {
            return; // Do nothing if the dialog was created without reset callbacks.
        }

        onResetStats();
        RefreshStatsLabels(getStats());
        Layout();
        Refresh();
    });
}

/**
 * @brief Refreshes all visible statistics labels.
 * @param stats Statistics snapshot that should be displayed.
 */
void ConnectFourStatsDialog::RefreshStatsLabels(const ConnectFourStats& stats)
{
    normalStatsLabel->SetLabel(FormatModeStats(stats.normalSinglePlayer));
    hardStatsLabel->SetLabel(FormatModeStats(stats.hardSinglePlayer));
    multiplayerStatsLabel->SetLabel(FormatModeStats(stats.multiplayer));
}
