#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/mediactrl.h>

class MenuPanel;
class SettingsPanel;

/**
 * @brief Main application window for the bundl. launcher.
 *
 * This class controls the main frame of the application, manages switching between the menu and settings screens, handles background music playback,
 * stores the current dark mode state, and launches the external mini games.
 *
 * @author Aditi Bhardwaj
 */
class MainFrame : public wxFrame
{
public:
    /**
     * @brief Constructs the main application frame.
     *
     * Initializes the frame, creates the menu and settings panels, loads background music, and prepares the launcher UI.
     */
    MainFrame();

    /**
     * @brief Shows the main menu panel.
     *
     * Hides the settings panel and refreshes the menu view.
     */
    void ShowMenu();

    /**
     * @brief Shows the settings panel.
     *
     * Hides the menu panel and refreshes the settings view.
     */
    void ShowSettings();

    /**
     * @brief Launches the selected mini game.
     *
     * Determines the correct game folder and executable based on the command, updates the working directory,
     * passes the dark mode setting, and starts the selected game.
     *
     * @param command The name of the game command to launch.
     */
    void LaunchGame(const wxString &command);

    /**
     * @brief Starts background music playback.
     *
     * Plays the loaded background audio if the media player is available.
     */
    void StartMusic();

    /**
     * @brief Stops background music playback.
     *
     * Stops the currently playing background audio if the media player exists.
     */
    void StopMusic();

    /**
     * @brief Enables or disables dark mode.
     *
     * Updates the stored theme state and applies the change to the frame and its child panels.
     *
     * @param enabled True to enable dark mode, false to disable it.
     */
    void SetDarkMode(bool enabled);

    /**
     * @brief Returns whether dark mode is currently enabled.
     *
     * @return true if dark mode is enabled, otherwise false.
     */
    bool IsDarkMode() const;

private:
    /**
     * @brief Loads the background music file for the launcher.
     *
     * Creates and prepares the media player using the configured audio asset.
     */
    void LoadBackgroundMusic();

    /**
     * @brief Handles the media finished event.
     *
     * Restarts or responds to music playback finishing depending on the player state and event flow.
     *
     * @param event The media event triggered when playback finishes.
     */
    void OnMusicFinished(wxMediaEvent &event);

    /**
     * @brief Builds a game executable if needed.
     *
     * Uses the target game folder and display name to attempt a build before launch.
     *
     * @param folder The folder containing the target game source files.
     * @param displayName The user-facing name of the game.
     */
    void BuildGame(const wxString &folder, const wxString &displayName);

    /**
     * @brief Prebuilds all bundled games.
     *
     * Attempts to build all supported mini games during launcher setup.
     */
    void PrebuildAllGames();

    /** @brief Pointer to the main menu panel. */
    MenuPanel *menuPanel;

    /** @brief Pointer to the settings panel. */
    SettingsPanel *settingsPanel;

    /** @brief Media controller used for background music playback. */
    wxMediaCtrl *mediaPlayer = nullptr;

    /** @brief Tracks whether dark mode is enabled. */
    bool darkMode = true;
};

#endif