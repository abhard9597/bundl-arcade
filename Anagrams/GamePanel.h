#pragma once
#include <wx/wx.h>
#include <functional>
#include <string>

/**
 * @file GamePanel.h
 * @brief Contains the GamePanel class which represents the main gameplay UI of the Anagrams game.
 * @date March 6th 2026
 * @author Manuelita Sowah
 */

/**
 * @class FlatButton
 * @brief A custom button class with flat design and hover/press effects.
 */
class FlatButton;

/**
 * @class GamePanel
 * @brief Represents the main gameplay UI of the Anagrams game.
 *
 * This class contains the UI elements for displaying the current rack, score, time left, status messages, and the list of found words. It also has input for submitting guesses and buttons for pausing and going back to the menu. The MainFrame will bind event handlers to this panel to handle user interactions.
 */
class GamePanel : public wxPanel
{
public:
    /**
     * @brief Constructs the GamePanel.
     * @param parent The parent window of this panel.
     */
    GamePanel(wxWindow *parent);

    /**
     * @brief Sets the callback for when the user wants to go back to the menu.
     * @param fn The function to call when the back action is triggered.
     */
    void SetOnBack(std::function<void()> fn) { m_onBack = std::move(fn); }

    /**
     * @brief Sets the callback for when the user wants to pause the game.
     * @param fn The function to call when the pause action is triggered.
     */
    void SetOnPause(std::function<void()> fn) { m_onPause = std::move(fn); }

    /**
     * @brief Sets the callback for when the user submits a guess.
     * @param fn The function to call with the submitted guess when the submit action is triggered.
     */
    void SetOnSubmit(std::function<void(const std::string &)> fn) { m_onSubmit = std::move(fn); }

    /**
     * @brief Updates the time left display.
     * @param secondsLeft The number of seconds left in the round.
     */
    void SetTime(int secondsLeft);

    /**
     * @brief Updates the score display.
     * @param score The current score.
     */
    void SetScore(int score);

    /**
     * @brief Updates the rack display.
     * @param rackSpacedUpper The string representing the current rack with spaces and uppercase letters.
     */
    void SetRack(const wxString &rackSpacedUpper);

    /**
     * @brief Updates the status message display.
     * @param status The status message to display.
     */
    void SetStatus(const wxString &status);

    /**
     * @brief Sets the callback for when the user wants to shuffle the rack.
     * @param fn The function to call when the shuffle action is triggered.
     */
    void SetOnShuffle(std::function<void()> fn) { m_onShuffle = std::move(fn); }

    /**
     * @brief Adds a found word to the list.
     * @param word The word to add.
     */
    void AddFoundWord(const wxString &word);

    /**
     * @brief Resets the UI for a new round.
     */
    void ResetRoundUI();

    /**
     * @brief Enables or disables gameplay elements.
     * @param enabled Whether to enable gameplay.
     */
    void EnableGameplay(bool enabled);

private:
    /**
     * @brief Handles the submission of a guess.
     */
    void Submit();

private:
    wxStaticText *m_timeLabel = nullptr;  ///< Display for time left
    wxStaticText *m_scoreLabel = nullptr; ///< Display for current score

    wxStaticText *m_rackLabel = nullptr;   ///< Display for current rack
    wxStaticText *m_statusLabel = nullptr; ///< Display for status messages

    wxTextCtrl *m_guessInput = nullptr; ///< Input field for the user's guess
    wxTextCtrl *m_foundList = nullptr;  ///< List box to display found words

    FlatButton *m_backBtn = nullptr;    ///< Button to go back to the menu
    FlatButton *m_pauseBtn = nullptr;   ///< Button to pause the game
    FlatButton *m_shuffleBtn = nullptr; ///< Button to shuffle the rack (optional, can be nullptr if not implemented)
    FlatButton *m_submitBtn = nullptr;  ///< Button to submit the guess

    std::function<void()> m_onBack, m_onPause, m_onShuffle; ///< Callbacks for back and pause actions
    std::function<void(const std::string &)> m_onSubmit;    ///< Callback for when a guess is submitted
};