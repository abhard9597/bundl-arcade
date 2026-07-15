#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <wx/string.h>
#include <wx/sound.h>

/**
 * @class SoundManager
 * @brief Handles short sound effects for the game.
 */
class SoundManager {
private:
    wxSound placeTokenSound;
    wxSound invalidMoveSound;
    wxSound winSound;
    wxSound loseSound;
    wxSound drawSound;
    wxSound menuClickSound;

    bool placeTokenLoaded;
    bool invalidMoveLoaded;
    bool winLoaded;
    bool loseLoaded;
    bool drawLoaded;
    bool menuClickLoaded;

public:
    /**
     * @brief Constructs the sound manager and loads sound files.
     */
    SoundManager();

    /**
     * @brief Plays the token placement sound.
     */
    void PlayPlaceTokenSound();

    /**
     * @brief Plays the invalid move sound.
     */
    void PlayInvalidMoveSound();

    /**
     * @brief Plays the win sound.
     */
    void PlayWinSound();

    /**
     * @brief Plays the lose sound.
     */
    void PlayLoseSound();

    /**
     * @brief Plays the draw sound.
     */
    void PlayDrawSound();

    /**
     * @brief Plays the menu button press sound.
     */
    void PlayMenuButtonSound();
};

#endif