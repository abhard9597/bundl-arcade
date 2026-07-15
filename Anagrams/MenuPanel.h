#pragma once
#include <wx/wx.h>
#include <functional>

/**
 * @file MenuPanel.h
 * @brief Contains the MenuPanel class which represents the main menu UI of the Anagrams game.
 * @date March 6th 2026
 * @author Manuelita Sowah
 */

/**
 * @class MenuPanel
 * @brief Represents the main menu UI of the Anagrams game.
 *
 * MenuPanel displays main menu buttons:
 * - New Game
 * - Load Game
 * - Tutorial
 * - Stats
 * - Quit
 */
class MenuPanel : public wxPanel
{
public:
    /**
     * @brief Constructs the MenuPanel.
     * @param parent The parent window of this panel.
     */
    MenuPanel(wxWindow *parent);

    /**
     * @brief Sets the callback for when the user wants to start a new game.
     * @param fn The function to call when the new game action is triggered.
     */
    void SetOnNewGame(std::function<void()> fn) { m_onNewGame = std::move(fn); }

    /**
     * @brief Sets the callback for when the user wants to view the tutorial.
     * @param fn The function to call when the tutorial action is triggered.
     */
    void SetOnTutorial(std::function<void()> fn) { m_onTutorial = std::move(fn); }

    /**
     * @brief Sets the callback for when the user wants to view statistics.
     * @param fn The function to call when the stats action is triggered.
     */
    void SetOnStats(std::function<void()> fn) { m_onStats = std::move(fn); }

    /**
     * @brief Sets the callback for when the user wants to quit the game.
     * @param fn The function to call when the quit action is triggered.
     */
    void SetOnQuit(std::function<void()> fn) { m_onQuit = std::move(fn); }

private:
    std::function<void()> m_onNewGame;  ///< Callback for starting a new game
    std::function<void()> m_onLoadGame; ///< Callback for loading a game
    std::function<void()> m_onTutorial; ///< Callback for viewing the tutorial
    std::function<void()> m_onStats;    ///< Callback for viewing statistics
    std::function<void()> m_onQuit;     ///< Callback for quitting the game
    wxBitmap m_bgOriginal;              ///< Original background image loaded from file
    wxBitmap m_bgScaled;                ///< Scaled background image that fits the current panel size
};