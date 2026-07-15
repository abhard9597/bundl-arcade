#include "AnagramsMainFrame.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/filefn.h>

#include <algorithm>
#include <random>

#include "RoundOverDialog.h"
#include "App.h"
#include "SoundManager.h"

AnagramsMainFrame::AnagramsMainFrame()
    : wxFrame(nullptr, wxID_ANY, "Anagrams", wxDefaultPosition, wxSize(900, 700)),
      m_timer(this)
{
    m_root = new wxPanel(this);
    m_rootSizer = new wxBoxSizer(wxVERTICAL);
    m_root->SetSizer(m_rootSizer);

    // Load dictionary (adjust path if needed)
    m_dict.LoadFromFile("words.txt");
    m_logic.SetDictionary(&m_dict);

    // ✅ LOCAL per-user/per-machine stats location
    wxString statsDir = wxStandardPaths::Get().GetUserLocalDataDir();
    if (!wxDirExists(statsDir)) {
        wxMkdir(statsDir);
    }
    wxString statsPath = statsDir + "/anagrams_stats.txt";
    m_stats.Load(statsPath.ToStdString());

    Bind(wxEVT_TIMER, &AnagramsMainFrame::OnTick, this);

    ShowMenu();
    Centre();
}

void AnagramsMainFrame::ShowMenu()
{
    m_timer.Stop();
    m_isPaused = false;

    m_rootSizer->Clear(true);

    m_menu = new MenuPanel(m_root);

    m_menu->SetOnNewGame([this]{ OnNewGame(); });
    m_menu->SetOnTutorial([this]{ OnTutorialFromMenu(); });
    m_menu->SetOnStats([this]{ OnStatsFromMenu(); });
    m_menu->SetOnQuit([this]{ Close(true); });

    m_rootSizer->Add(m_menu, 1, wxEXPAND);
    m_root->Layout();
}

void AnagramsMainFrame::ShowGame()
{
    m_rootSizer->Clear(true);

    m_game = new GamePanel(m_root);

    m_game->SetOnBack([this]{ OnBackToMenu(); });
    m_game->SetOnPause([this]{ OnPause(); });
    m_game->SetOnShuffle([this]{ OnShuffle(); });
    m_game->SetOnSubmit([this](const std::string& w){ OnSubmitWord(w); });

    m_rootSizer->Add(m_game, 1, wxEXPAND);
    m_root->Layout();
}

void AnagramsMainFrame::OnNewGame()
{
    ShowGame();
    StartRound();
}

void AnagramsMainFrame::OnTutorialFromMenu()
{
    PauseDialog dlg(
        this,
        PauseDialog::Tab::Tutorial,
        /*showResume=*/false,
        m_stats.Get(),
        [this]{ return m_stats.Get(); },   // getter (for refresh)
        [this]{ m_stats.Reset(); }         // reset
    );
    dlg.ShowModal();
}

void AnagramsMainFrame::OnStatsFromMenu()
{
    PauseDialog dlg(
        this,
        PauseDialog::Tab::Stats,
        /*showResume=*/false,
        m_stats.Get(),
        [this]{ return m_stats.Get(); },   // getter (for refresh)
        [this]{ m_stats.Reset(); }         // reset
    );
    dlg.ShowModal();
}

void AnagramsMainFrame::StartRound()
{
    m_secondsLeft = 60;
    m_isPaused = false;

    // reset score/found words
    m_logic.ResetRound();

    // Pick a 6-letter base word and shuffle to create rack
    std::string base = m_dict.RandomWordOfLength(6);
    if (base.empty()) base = "aetrns"; // fallback rack

    static std::random_device rd;
    static std::mt19937 rng(rd());

    std::string rack = base;
    for (int tries = 0; tries < 20; ++tries) {
        std::shuffle(rack.begin(), rack.end(), rng);
        if (rack != base) break;
    }

    m_logic.SetRack(rack);

    if (m_game) {
        m_game->ResetRoundUI();
        m_game->SetRack(m_logic.GetRackSpacedUpper());
        m_game->SetScore(m_logic.GetScore());
        m_game->SetTime(m_secondsLeft);
        m_game->SetStatus("Make as many words as you can!");
        m_game->EnableGameplay(true);
    }

    m_timer.Start(1000);
}

void AnagramsMainFrame::EndRound()
{
    m_timer.Stop();
    if (m_game) m_game->EnableGameplay(false);

    // ✅ Update persistent stats
    m_stats.RecordRound(
        m_logic.GetScore(),
        m_logic.GetWordsFoundCount(),
        60
    );

    // game over SFX
    wxGetApp().GetSfx().PlayTimesUp();

    // missed words (valid but not found)
    std::string rack = m_logic.GetRack();
    auto allValid = m_dict.AllWordsFromRack(rack, 3, 6);
    auto missed = m_logic.GetMissedWords(allValid);

    RoundOverDialog dlg(this, m_logic.GetScore(), missed);
    int choice = dlg.ShowModal();

    if (choice == RoundOverDialog::PlayAgain) {
        StartRound();
    } else {
        ShowMenu();
    }
}

void AnagramsMainFrame::OnBackToMenu()
{
    ShowMenu();
}

void AnagramsMainFrame::OnPause()
{
    if (m_isPaused) return;

    m_isPaused = true;
    m_timer.Stop();
    if (m_game) m_game->EnableGameplay(false);

    PauseDialog dlg(
        this,
        PauseDialog::Tab::Tutorial,
        /*showResume=*/true,
        m_stats.Get(),
        [this]{ return m_stats.Get(); },   // getter (for refresh)
        [this]{ m_stats.Reset(); }         // reset
    );
    dlg.ShowModal();

    m_isPaused = false;
    if (m_game) m_game->EnableGameplay(true);
    if (m_secondsLeft > 0) m_timer.Start(1000);
}

void AnagramsMainFrame::OnShuffle()
{
    std::string rack = m_logic.GetRack();
    if (rack.empty()) return;

    static std::random_device rd;
    static std::mt19937 rng(rd());

    std::shuffle(rack.begin(), rack.end(), rng);
    m_logic.SetRack(rack);

    if (m_game) m_game->SetRack(m_logic.GetRackSpacedUpper());
}

void AnagramsMainFrame::OnSubmitWord(const std::string& word)
{
    auto res = m_logic.TrySubmit(word);

    if (!res.accepted) {
        wxGetApp().GetSfx().PlayWrong();
        if (m_game) m_game->SetStatus(res.message);
        return;
    }

    wxGetApp().GetSfx().PlayCorrect();

    if (m_game) {
        m_game->AddFoundWord(wxString::Format("%s  (+%d)", res.displayWord, res.pointsGained));
        m_game->SetScore(m_logic.GetScore());
        m_game->SetStatus(res.message);
    }
}

void AnagramsMainFrame::OnTick(wxTimerEvent&)
{
    if (m_isPaused) return;

    if (m_secondsLeft > 0) {
        m_secondsLeft--;
        if (m_game) m_game->SetTime(m_secondsLeft);

        if (m_secondsLeft == 0) {
            EndRound();
        }
    }
}
