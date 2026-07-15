/**
 * @file ConnectFourPanel.h
 * @brief Declares the Connect Four gameplay panel.
 *
 * This header defines the panel responsible for drawing the game board, handling
 * input, coordinating AI turns, and recording finished-game statistics.
 *
 * @author Lucas Brown and collaborators
 */

#ifndef CONNECTFOURPANEL_H
#define CONNECTFOURPANEL_H

#include <wx/wx.h>
#include <wx/timer.h>
#include "ConnectFourGame.h"
#include "ConnectFourMenu.h"
#include "ConnectFourAI.h"
#include "SoundManager.h"
#include "ConnectFourStatsManager.h"

/**
 * @class ConnectFourPanel
 * @brief Graphics panel for rendering and interacting with the Connect Four game.
 */
class ConnectFourPanel : public wxPanel
{
private:
    ConnectFourGame game; // game logic and state
    GameMode mode;        // selected game mode (single player or multiplayer)
    ConnectFourAI ai;     // AI logic for single player mode

    wxTimer aiTimer; // timer used to delay AI moves
    bool aiThinking; // indicates whether the AI is currently "thinking"
    bool statsRecordedForCurrentGame; // prevents the same finished game from being counted twice after the dialog is shown

    // original and scaled versions of the themed background image
    wxBitmap m_bgOriginal;
    wxBitmap m_bgScaled;

    SoundManager soundManager; // handles short sound effects

    static const int CELL_SIZE = 82; // size of each cell in pixels
    static const int PADDING = 20;   // padding around the board

    /**
     * @brief Handles paint events.
     * @param event paint event
     */
    void OnPaint(wxPaintEvent &event);

    /**
     * @brief Handles mouse click events.
     * @param event mouse click event
     */
    void OnLeftClick(wxMouseEvent &event);

    /**
     * @brief Handles resize events so the background can be rescaled.
     * @param event size event
     */
    void OnSize(wxSizeEvent &event);

    /**
     * @brief Handles AI timer events. Triggered after a delay to allow the AI to make its move.
     * @param event timer event
     */
    void OnAITimer(wxTimerEvent &event);

    /**
     * @brief Helper that calculates the board's drawing position.
     * @return wxRect rectangle for the board area
     */
    wxRect GetBoardRect() const;

    /**
     * @brief Executes the AI's move.
     */
    void MakeAIMove();

    /**
     * @brief Displays the game-over dialog.
     */
    void ShowGameOverDialog();

    /**
     * @brief Records the finished game in the local statistics file if it has not been counted yet.
     *
     * This helper prevents duplicate recordings when the game-over dialog is
     * reopened or when the panel refreshes after the result has already been
     * saved.
     */
    void RecordStatsIfNeeded();

public:
    /**
     * @brief Constructs the Connect Four panel. Initializes the panel with the selected game mode and AI difficulty.
     * @param parent parent window
     * @param selectedMode selected game mode
     * @param difficulty selected AI difficulty
     */
    ConnectFourPanel(wxWindow *parent, GameMode selectedMode, AIDifficulty difficulty = AIDifficulty::None);

    /**
     * @brief Draws the game board and pieces.
     * @param dc device context used for drawing
     */
    void DrawBoard(wxDC &dc);

    wxDECLARE_EVENT_TABLE();
};

#endif