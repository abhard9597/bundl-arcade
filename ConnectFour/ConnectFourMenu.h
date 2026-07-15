/**
 * @file ConnectFourMenu.h
 * @brief Declares the Connect Four menu dialog and related menu enums.
 *
 * This header contains the menu state enums and the ConnectFourMenu class used
 * to start games, choose single-player difficulty, open the statistics window,
 * and close the application.
 *
 * @author Lucas Brown and collaborators
 */

#ifndef CONNECTFOURMENU_H
#define CONNECTFOURMENU_H

#include <wx/wx.h>
#include "ConnectFourAI.h"
#include "SoundManager.h"
#include "ConnectFourStatsDialog.h"
#include "ConnectFourStatsManager.h"

/**
 * @enum GameMode
 * @brief Represents the available game modes.
 */
enum class GameMode {
    None, // no mode selected
    MultiPlayer, // two human players
    SinglePlayer // player vs AI
};

/**
 * @enum MenuScreen
 * @brief Represents the current menu screen.
 */
enum class MenuScreen {
    Main, // main menu (mode selection)
    Difficulty // difficulty selection menu
};

/**
 * @class ConnectFourMenu
 * @brief Dialog window for selecting game mode and difficulty.
 * This class provides a themed menu interface allowing the user to:
 * - choose between single player and multiplayer modes
 * - select AI difficulty (for single player)
 * - exit the application
 */
class ConnectFourMenu : public wxDialog {
private:
    GameMode selectedMode; // selected game mode
    AIDifficulty selectedDifficulty; // selected AI difficulty
    MenuScreen currentScreen; // current menu screen state

    wxStaticText* title; // title text displayed at the top
    wxStaticText* subtitle; // subtitle text under the title

    wxPanel* button1; // first custom menu button
    wxPanel* button2; // second custom menu button
    wxPanel* button3; // third custom menu button
    wxPanel* button4; // fourth custom menu button used for statistics or quitting depending on the screen

    // original and scaled versions of the background image
    wxBitmap m_bgOriginal;
    wxBitmap m_bgScaled;

    SoundManager soundManager; // handles menu sound effects

    /**
     * @brief Displays the main menu screen.
     */
    void ShowMainMenu();

    /**
     * @brief Displays the difficulty selection screen.
     */
    void ShowDifficultyMenu();

    /**
     * @brief Updates the text shown on one of the custom menu buttons.
     * @param button button panel to update
     * @param titleText main button title
     * @param subtitleText smaller descriptive subtitle
     */
    void SetButtonText(wxPanel* button, const wxString& titleText, const wxString& subtitleText);

    /**
     * @brief Handles button 1 click events.
     * @param event button click event
     */
    void OnButton1(wxCommandEvent& event);

    /**
     * @brief Handles button 2 click events.
     * @param event button click event
     */
    void OnButton2(wxCommandEvent& event);

    /**
     * @brief Handles button 3 click events.
     * @param event button click event
     */
    void OnButton3(wxCommandEvent& event);

    /**
     * @brief Handles button 4 click events.
     *
     * On the current menu layout, this button is used to quit the dialog from
     * both the main menu and the difficulty selection screen.
     *
     * @param event button click event
     */
    void OnButton4(wxCommandEvent& event);

    /**
     * @brief Handles background painting.
     * @param event paint event
     */
    void OnPaint(wxPaintEvent& event);

    /**
     * @brief Handles resize events so the background can rescale.
     * @param event size event
     */
    void OnSize(wxSizeEvent& event);

public:
    /**
     * @brief Constructs the Connect Four menu dialog.
     * @param parent parent window
     */
    ConnectFourMenu(wxWindow* parent);

    /**
     * @brief Gets the selected game mode.
     * @return GameMode selected mode
     */
    GameMode getSelectedMode() const;

    /**
     * @brief Gets the selected AI difficulty
     * @return AIDifficulty selected difficulty
     */
    AIDifficulty getSelectedDifficulty() const;
};

#endif