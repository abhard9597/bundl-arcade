#include "SoundManager.h"
#include <wx/filefn.h>
#include <wx/utils.h>

bool SoundManager::Init(const wxString& exeDir)
{
    wxString sfxDir = exeDir + "/addFiles/sfx";

    m_clickPath   = sfxDir + "/menu_click.wav";
    m_correctPath = sfxDir + "/word_entered.wav";
    m_wrongPath   = sfxDir + "/error.wav";
    m_timesupPath = sfxDir + "/game_over.wav";

    return wxFileExists(m_clickPath);
}

void SoundManager::PlayFile(const wxString& path)
{
    if (!m_enabled) return;
    if (path.empty() || !wxFileExists(path)) return;

#if defined(__APPLE__)
    // macOS: afplay
    wxCharBuffer c0 = wxString("/usr/bin/afplay").utf8_str();
    wxCharBuffer c1 = path.utf8_str();
    const char* argv[] = { c0.data(), c1.data(), nullptr };
    wxExecute(argv, wxEXEC_ASYNC);

#elif defined(__linux__)
    // Linux: try paplay, then aplay
    if (wxFileExists("/usr/bin/paplay") || wxFileExists("/bin/paplay")) {
        wxCharBuffer c0 = wxString("paplay").utf8_str();
        wxCharBuffer c1 = path.utf8_str();
        const char* argv[] = { c0.data(), c1.data(), nullptr };
        wxExecute(argv, wxEXEC_ASYNC);
    } else if (wxFileExists("/usr/bin/aplay") || wxFileExists("/bin/aplay")) {
        wxCharBuffer c0 = wxString("aplay").utf8_str();
        wxCharBuffer c1 = path.utf8_str();
        const char* argv[] = { c0.data(), c1.data(), nullptr };
        wxExecute(argv, wxEXEC_ASYNC);
    } else {
        // no player available -> silently do nothing
    }

#elif defined(_WIN32)
    // Windows: simplest option is disable (or use PlaySound from WinAPI).
    // For now: do nothing (no crash).
    (void)path;
    return;

#else
    (void)path;
    return;
#endif
}

void SoundManager::PlayClick()   { PlayFile(m_clickPath); }
void SoundManager::PlayCorrect() { PlayFile(m_correctPath); }
void SoundManager::PlayWrong()   { PlayFile(m_wrongPath); }
void SoundManager::PlayTimesUp() { PlayFile(m_timesupPath); }