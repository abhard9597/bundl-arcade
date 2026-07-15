/**
 * @file ConnectFourStatsDialog.h
 * @brief Declares the statistics dialog used by Connect Four.
 *
 * This file defines a small wxDialog that displays saved statistics for normal
 * single player, hard single player, and multiplayer. It can also refresh the
 * displayed values and trigger a reset through callback functions.
 *
 */

#ifndef CONNECTFOURSTATSDIALOG_H
#define CONNECTFOURSTATSDIALOG_H

#include <functional>

#include <wx/wx.h>

#include "ConnectFourStatsManager.h"

/**
 * @class ConnectFourStatsDialog
 * @brief Displays saved Connect Four statistics in a themed dialog window.
 *
 * The dialog shows three sections of locally stored results and provides a
 * reset button that can clear the saved file through a callback supplied by the
 * menu. The class only handles presentation and relies on the manager for file
 * operations.
 *
 */
class ConnectFourStatsDialog : public wxDialog
{
public:
    /**
     * @brief Constructs the statistics dialog.
     *
     * The dialog starts with a supplied snapshot of the current statistics and
     * can optionally receive callbacks for reloading the latest values and for
     * resetting the file when the user presses the reset button.
     *
     * @param parent Parent window that owns this dialog.
     * @param statsSnapshot Statistics snapshot used to initialize the labels.
     * @param getStats Callback used to fetch a fresh statistics snapshot after a reset.
     * @param onResetStats Callback used to clear the saved statistics.
     */
    ConnectFourStatsDialog(wxWindow* parent,
                           const ConnectFourStats& statsSnapshot,
                           std::function<ConnectFourStats()> getStats = {},
                           std::function<void()> onResetStats = {});

private:
    /**
     * @brief Updates the text labels shown in the dialog.
     *
     * Each label is refreshed with the newest values for its matching game mode.
     *
     * @param stats Statistics snapshot to display.
     */
    void RefreshStatsLabels(const ConnectFourStats& stats);

    wxStaticText* normalStatsLabel = nullptr;      
    wxStaticText* hardStatsLabel = nullptr;        
    wxStaticText* multiplayerStatsLabel = nullptr; 
};

#endif
