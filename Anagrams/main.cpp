#include <wx/wx.h>
#include <wx/image.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "AnagramsMainFrame.h"
#include "SoundManager.h"
#include "App.h"

// One global SFX manager for the whole app (wxSound and wxMediaCtrl are very limited, so we use system calls to afplay instead)
static SoundManager g_sfx;

SoundManager &MyApp::GetSfx() { return g_sfx; }

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();

    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxFileName exeFile(exePath);
    wxString exeDir = exeFile.GetPath();

    g_sfx.Init(exeDir);

    auto *frame = new AnagramsMainFrame();
    frame->Show();

    return true;
}

wxIMPLEMENT_APP(MyApp);