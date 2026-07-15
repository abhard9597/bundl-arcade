#pragma once
#include <wx/wx.h>
#include <wx/simplebook.h>
#include <functional>

#include "StatsManager.h" // GameStats

/**
 * @class PauseDialog
 * @brief Represents the pause menu UI of the Anagrams game.
 *
 * This dialog is shown when the user pauses the game or opens Tutorial/Stats from the main menu.
 * It contains a notebook with two tabs: Tutorial and Statistics. It also optionally shows a
 * Resume button (when launched from gameplay).
 *
 * The dialog displays a snapshot of current statistics and can optionally expose a "Reset Stats"
 * action through a callback.
 */
class PauseDialog : public wxDialog
{
public:
    /**
     * @enum Tab
     * @brief Represents the tabs in the pause dialog.
     */
    enum class Tab
    {
        Tutorial = 0,
        Stats = 1
    };

    /**
     * @brief Constructs the PauseDialog.
     * @param parent The parent window of this dialog.
     * @param startTab The tab to show when the dialog is opened.
     * @param showResume Whether to show the resume button (if false, the dialog will only have a close button).
     * @param statsSnapshot Snapshot of game statistics to display.
     * @param getStats Optional callback to get the latest stats, used to refresh the stats
     * @param onResetStats Optional callback invoked when the user chooses to reset stats.
     */
    PauseDialog(wxWindow *parent,
                Tab startTab,
                bool showResume,
                const GameStats &statsSnapshot,
                std::function<GameStats()> getStats = {},
                std::function<void()> onResetStats = {});

private:
    wxSimplebook *m_book = nullptr;
};