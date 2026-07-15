#pragma once
#include <wx/sound.h>
#include <wx/string.h>

/**
 * @file SoundManager.h
 * @brief Contains the SoundManager class which manages the loading and playing of sound effects in the Anagrams game.
 * @date March 30th 2026
 * @author Manuelita Sowah
 */

/**
 * @class SoundManager
 * @brief A class responsible for loading and playing sound effects in the Anagrams game. It
 */
class SoundManager
{
public:
    /**
     * @brief Initializes the SoundManager by loading all necessary sound effects. This should be called at the start of the application.
     * @param parent The parent window, used for any necessary context when loading sounds.
     * @param exeDir The directory where the executable is located, used to find the sound files.
     * @return true if initialization was successful, false otherwise.
     */
    bool Init(const wxString &exeDir);

    /**
     * @brief Enables or disables sound effects. When disabled, calls to play sounds will have no effect.
     * @param enabled Set to true to enable sounds, false to disable.
     */
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * @brief Checks whether sound effects are currently enabled.
     * @return true if sounds are enabled, false otherwise.
     */
    bool IsEnabled() const { return m_enabled; }

    /**
     * @brief Plays the click sound effect, typically used for button clicks.
     */
    void PlayClick();

    /**
     * @brief Plays the correct sound effect, typically used when the player submits a valid word.
     */
    void PlayCorrect();

    /**
     * @brief Plays the wrong sound effect, typically used when the player submits an invalid word.
     */
    void PlayWrong();

    /**
     * @brief Plays the times up sound effect, typically used when the timer runs out.
     */
    void PlayTimesUp();

private:
    /**
     * @brief Helper function to play a given sound effect if sounds are enabled.
     * @param path The path to the sound file to play.
     */
    void PlayFile(const wxString& path);

private:
    bool m_enabled = true;                          ///< Indicates whether sound effects are enabled or disabled
    wxString m_clickPath; ///< The path to the sound file for button clicks
    wxString m_correctPath; ///< The path to the sound file for correct word submissions
    wxString m_wrongPath; // The path to the sound file for wrong word submissions
    wxString m_timesupPath; ///< The path to the sound file for when the timer runs out
};