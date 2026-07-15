#pragma once
#include <wx/wx.h>
#include <wx/timer.h>

#include "MenuPanel.h"
#include "GamePanel.h"
#include "GameLogic.h"
#include "PauseDialog.h"
#include "Dictionary.h"

/**
 * @file MainFrame.h
 * @brief Contains the MainFrame class which is the main window of the Anagrams game.
 * @date March 6th 2026
 * @author Manuelita Sowah
 */

/**
 * @class MainFrame
 * @brief The main window of the Anagrams game.
 *
 * This class manages the main UI components of the game, including the menu, game panel, and pause dialog. It also handles the main game loop through a timer and processes user interactions.
 */
class AnagramsMainFrame : public wxFrame
{

public:
    /**
     * @brief Constructs the MainFrame window.
     * This initializes the main frame, sets up the root panel and sizer, binds the timer event, and shows the main menu.
     */
    AnagramsMainFrame();

private:
    /**
     * @brief Shows the main menu panel.
     * This function clears the current UI, creates a new MenuPanel, binds its events, and lays it out.
     */
    void ShowMenu();

    /**
     * @brief Shows the game panel.
     * This function clears the current UI, creates a new GamePanel, binds its events, and lays it out.
     */
    void ShowGame();

    /**
     * @brief Starts a new round of the game.
     * This resets the game logic, updates the UI for a new round, and starts the
     */
    void StartRound();

    /**
     * @brief Ends the current round of the game.
     * This stops the timer, shows the end-of-round dialog with stats, and then returns to the main menu.
     */
    void EndRound();

    // Event handlers

    /**
     * @brief Handles timer ticks for the game loop.
     * @param e The timer event.
     */
    void OnTick(wxTimerEvent &e);

    /**
     * @brief Handles the "New Game" action from the menu.
     * This starts a new game by showing the game panel and starting a new round.
     */
    void OnNewGame();

    /**
     * @brief Handles the "Tutorial" action from the menu.
     * This displays the tutorial dialog.
     */
    void OnTutorialFromMenu();

    /**
     * @brief Handles the "Stats" action from the menu.
     * This displays the stats dialog.
     */
    void OnStatsFromMenu();

    /**
     * @brief Handles the "Back to Menu" action from the game panel.
     * This returns the user to the main menu.
     */
    void OnBackToMenu();

    /**
     * @brief Handles the "Pause" action from the game panel.
     * This pauses the game, shows the pause dialog, and resumes when the dialog is closed.
     */
    void OnPause();

    /**
     * @brief Handles the submission of a word from the game panel.
     * @param word The word submitted by the user.
     * This processes the submitted word through the game logic and updates the UI accordingly.
     */
    void OnSubmitWord(const std::string &word);

    /**
     * @brief Handles the "Shuffle" action from the game panel.
     * This shuffles the current rack of letters to help the user see new possibilities.
     */
    void OnShuffle();

    /**
     * @brief Handles the "Pause" keyboard shortcut.
     * @param e The command event triggered by the accelerator.
     */
    void OnAccelPause(wxCommandEvent &e);

private:
    wxPanel *m_root = nullptr;         ///< The root panel that holds the current view (menu or game)
    wxBoxSizer *m_rootSizer = nullptr; ///< The sizer for the root panel to manage layout

    MenuPanel *m_menu = nullptr; ///< The menu panel, shown when the user is in the main menu
    GamePanel *m_game = nullptr; ///< The game panel, shown when the user is playing a round

    wxTimer m_timer;         ///< The timer used for the game loop to track time left in the round
    int m_secondsLeft = 60;  ///< The number of seconds left in the current round
    bool m_isPaused = false; ///< Whether the game is currently paused

    GameLogic m_logic; ///< The game logic instance that manages the game state and rules

    Dictionary m_dict; ///< The dictionary instance used for validating submitted words

    StatsManager m_stats; ///< The stats manager for recording and retrieving game statistics
};