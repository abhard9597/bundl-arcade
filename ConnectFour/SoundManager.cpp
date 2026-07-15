#include "SoundManager.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>

namespace {
    /**
     * @brief Looks for a sound file in a few likely locations.
     * @param fileName name of the file to search for
     * @return wxString path to the file if found, otherwise an empty string
     */
    wxString FindAssetPath(const wxString& fileName) {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exeFile(exePath);
        wxString exeDir = exeFile.GetPath();

        wxArrayString candidates;
        candidates.Add(exeDir + "/" + fileName);
        candidates.Add(exeDir + "/addFiles/" + fileName);
        candidates.Add(fileName);
        candidates.Add("addFiles/" + fileName);

        for (const auto& path : candidates) {
            if (wxFileExists(path)) {
                return path;
            }
        }

        return "";
    }
}

/**
 * @brief Constructs the sound manager and loads the sound files.
 */
SoundManager::SoundManager()
    : placeTokenLoaded(false),
      invalidMoveLoaded(false),
      winLoaded(false),
      loseLoaded(false),
      drawLoaded(false),
      menuClickLoaded(false)
{
    wxString placeTokenPath = FindAssetPath("Place_token.wav");
    wxString invalidMovePath = FindAssetPath("Invalid_move.wav");
    wxString winPath = FindAssetPath("Win_sound.wav");
    wxString losePath = FindAssetPath("Lose_sound.wav");
    wxString drawPath = FindAssetPath("Draw_sound.wav");

    if (!placeTokenPath.IsEmpty()) {
        placeTokenSound.Create(placeTokenPath);
        placeTokenLoaded = placeTokenSound.IsOk();

        // Reuse the same sound for menu button clicks
        menuClickSound.Create(placeTokenPath);
        menuClickLoaded = menuClickSound.IsOk();
    }

    if (!invalidMovePath.IsEmpty()) {
        invalidMoveSound.Create(invalidMovePath);
        invalidMoveLoaded = invalidMoveSound.IsOk();
    }

    if (!winPath.IsEmpty()) {
        winSound.Create(winPath);
        winLoaded = winSound.IsOk();
    }

    if (!losePath.IsEmpty()) {
        loseSound.Create(losePath);
        loseLoaded = loseSound.IsOk();
    }

    if (!drawPath.IsEmpty()) {
        drawSound.Create(drawPath);
        drawLoaded = drawSound.IsOk();
    }
}

/**
 * @brief Plays the token placement sound.
 */
void SoundManager::PlayPlaceTokenSound() {
    if (placeTokenLoaded) {
        placeTokenSound.Play(wxSOUND_ASYNC);
    }
}

/**
 * @brief Plays the invalid move sound.
 */
void SoundManager::PlayInvalidMoveSound() {
    if (invalidMoveLoaded) {
        invalidMoveSound.Play(wxSOUND_ASYNC);
    }
}

/**
 * @brief Plays the win sound.
 */
void SoundManager::PlayWinSound() {
    if (winLoaded) {
        winSound.Play(wxSOUND_ASYNC);
    }
}

/**
 * @brief Plays the lose sound.
 */
void SoundManager::PlayLoseSound() {
    if (loseLoaded) {
        loseSound.Play(wxSOUND_ASYNC);
    }
}

/**
 * @brief Plays the draw sound.
 */
void SoundManager::PlayDrawSound() {
    if (drawLoaded) {
        drawSound.Play(wxSOUND_ASYNC);
    }
}

/**
 * @brief Plays the menu click sound.
 */
void SoundManager::PlayMenuButtonSound() {
    if (menuClickLoaded) {
        menuClickSound.Play(wxSOUND_ASYNC);
    }
}
