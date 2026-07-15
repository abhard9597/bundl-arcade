/**
 * @file ConnectFourPanel.cpp
 * @brief Implements the Connect Four gameplay panel.
 *
 * This file handles drawing, player clicks, AI turns, end-of-game prompts, and
 * statistics recording for Connect Four.
 *
 * @author Lucas Brown and collaborators
 */

#include "ConnectFourPanel.h"
#include "ConnectFourAppTheme.h"

#include <wx/dcbuffer.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/filefn.h>

namespace
{
    const AppTheme::Palette THEME = AppTheme::GetPalette();

    const wxColour BG_FALLBACK = THEME.panelBg;
    const wxColour PANEL_OVERLAY = AppTheme::IsDarkMode()
                                       ? wxColour(0, 0, 0, 55)
                                       : wxColour(255, 255, 255, 35);

    const wxColour TITLE_TEXT = THEME.text;
    const wxColour SUBTITLE_TEXT = THEME.subtleText;

    const wxColour CARD = THEME.card;
    const wxColour CARD_EDGE = THEME.line;
    const wxColour SHADOW = THEME.shadow;

    const wxColour BOARD_BODY = AppTheme::IsDarkMode()
                                    ? wxColour(34, 58, 82)
                                    : wxColour(205, 221, 236);

    const wxColour BOARD_EDGE = AppTheme::IsDarkMode()
                                    ? wxColour(185, 215, 232)
                                    : wxColour(120, 150, 180);

    const wxColour EMPTY_SLOT = AppTheme::IsDarkMode()
                                    ? wxColour(234, 241, 245)
                                    : wxColour(250, 252, 255);

    const wxColour EMPTY_SLOT_EDGE = AppTheme::IsDarkMode()
                                         ? wxColour(175, 190, 198)
                                         : wxColour(180, 190, 200);

    const wxColour PLAYER_ONE(230, 102, 92);
    const wxColour PLAYER_ONE_EDGE(170, 70, 62);

    const wxColour PLAYER_TWO(241, 198, 94);
    const wxColour PLAYER_TWO_EDGE(180, 145, 58);

    /**
     * @brief Looks for an asset file in a few likely locations.
     * @param fileName name of the file to search for
     * @return wxString path to the file if found, otherwise an empty string
     */
    wxString FindAssetPath(const wxString &fileName)
    {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exeFile(exePath);
        wxString exeDir = exeFile.GetPath();

        wxArrayString candidates;
        candidates.Add(exeDir + "/" + fileName);
        candidates.Add(exeDir + "/addFiles/" + fileName);
        candidates.Add(fileName);
        candidates.Add("addFiles/" + fileName);

        for (const auto &path : candidates)
        {
            if (wxFileExists(path))
            {
                return path;
            }
        }

        return "";
    }
}

/**
 * @brief Event table for ConnectFourPanel.
 * Maps paint and mouse click events to their corresponding functions.
 */
wxBEGIN_EVENT_TABLE(ConnectFourPanel, wxPanel)
    EVT_PAINT(ConnectFourPanel::OnPaint)
        EVT_LEFT_DOWN(ConnectFourPanel::OnLeftClick)
            wxEND_EVENT_TABLE()

    /**
     * @brief Constructs the Connect Four game panel.
     * Initializes the panel, stores the selected game mode and AI difficulty, loads the background image,
     * and binds the timer and resize events.
     * @param parent parent window
     * @param selectedMode selected game mode
     * @param difficulty selected AI difficulty
     */
    ConnectFourPanel::ConnectFourPanel(wxWindow *parent, GameMode selectedMode, AIDifficulty difficulty)
    : wxPanel(parent), mode(selectedMode), ai(difficulty), aiTimer(this), aiThinking(false), statsRecordedForCurrentGame(false)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(BG_FALLBACK);

    // load the same background image used by the menu
    wxString bgPath = FindAssetPath("menu_bg.png");
    if (!bgPath.IsEmpty())
    {
        wxImage img;
        if (img.LoadFile(bgPath))
        {
            m_bgOriginal = wxBitmap(img);
        }
    }

    Bind(wxEVT_TIMER, &ConnectFourPanel::OnAITimer, this); // bind the timer event so when the delay finishes, the AI can make its move
    Bind(wxEVT_SIZE, &ConnectFourPanel::OnSize, this);     // resize background cleanly when the window size changes
}

/**
 * @brief Handles paint events for the panel.
 * Draws the themed background first, then draws the board and status area.
 * @param event paint event
 */
void ConnectFourPanel::OnPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxSize sz = GetClientSize();

    // draw the same background image used by the menu so the whole game feels connected
    if (m_bgOriginal.IsOk())
    {
        if (!m_bgScaled.IsOk() ||
            m_bgScaled.GetWidth() != sz.GetWidth() ||
            m_bgScaled.GetHeight() != sz.GetHeight())
        {
            wxImage scaled = m_bgOriginal.ConvertToImage();
            scaled.Rescale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH);
            m_bgScaled = wxBitmap(scaled);
        }

        dc.DrawBitmap(m_bgScaled, 0, 0, false);

        // overlay helps readability in both dark and light mode
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(PANEL_OVERLAY));
        dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
    }
    else
    {
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(BG_FALLBACK));
        dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
    }

    DrawBoard(dc);
}

/**
 * @brief Handles resize events for the panel.
 * Clears the cached scaled image so it can be rebuilt at the new size.
 * @param event size event
 */
void ConnectFourPanel::OnSize(wxSizeEvent &event)
{
    m_bgScaled = wxBitmap();
    Refresh();
    event.Skip();
}

/**
 * @brief Calculates where the board should be drawn on the panel.
 * Centers the board horizontally and leaves room at the top for the title and status card.
 * @return wxRect rectangle describing the board's position and size
 */
wxRect ConnectFourPanel::GetBoardRect() const
{
    int cols = game.getBoard().getColumns();
    int rows = game.getBoard().getRows();

    int boardWidth = cols * CELL_SIZE;
    int boardHeight = rows * CELL_SIZE;

    int x = (GetClientSize().GetWidth() - boardWidth) / 2;
    int y = 175;

    if (x < 30)
    {
        x = 30;
    }

    return wxRect(x, y, boardWidth, boardHeight);
}

/**
 * @brief Draws the Connect Four board and status text.
 * Renders the themed title, status card, board frame, and all game pieces.
 * @param dc device context used for drawing
 */
void ConnectFourPanel::DrawBoard(wxDC &dc)
{
    const ConnectFourBoard &board = game.getBoard();
    wxRect boardRect = GetBoardRect();
    wxSize sz = GetClientSize();

    // main title
    dc.SetTextForeground(TITLE_TEXT);
    dc.SetFont(wxFontInfo(28).Bold().FaceName("Avenir Next"));
    dc.DrawText("Connect Four", 30, 24);

    dc.SetTextForeground(SUBTITLE_TEXT);
    dc.SetFont(wxFontInfo(12).FaceName("Avenir Next"));
    dc.DrawText("Drop a piece into a column and connect four in a row", 32, 62);

    // status card near the top of the window
    wxRect statusRect(30, 92, sz.GetWidth() - 60, 62);

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(SHADOW));
    dc.DrawRoundedRectangle(statusRect.x + 5, statusRect.y + 6, statusRect.width, statusRect.height, 16);

    dc.SetBrush(wxBrush(CARD));
    dc.DrawRoundedRectangle(statusRect.x, statusRect.y, statusRect.width, statusRect.height, 16);

    dc.SetPen(wxPen(CARD_EDGE, 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRoundedRectangle(statusRect.x, statusRect.y, statusRect.width, statusRect.height, 16);

    wxString modeText = (mode == GameMode::SinglePlayer) ? "Mode: Single Player" : "Mode: Multiplayer";
    wxString statusText;

    if (!game.getIsGameOver())
    {
        if (mode == GameMode::SinglePlayer)
        {
            if (aiThinking)
            {
                statusText = "Computer is thinking...";
            }
            else if (game.getCurrentPlayer() == 1)
            {
                statusText = "Your turn";
            }
            else
            {
                statusText = "Computer's turn";
            }
        }
        else
        {
            statusText = wxString::Format("Player %d's turn", game.getCurrentPlayer());
        }
    }
    else if (game.getWinner() == 0)
    {
        statusText = "Draw game!";
    }
    else if (mode == GameMode::SinglePlayer)
    {
        if (game.getWinner() == 1)
        {
            statusText = "You win!";
        }
        else
        {
            statusText = "Computer wins!";
        }
    }
    else
    {
        statusText = wxString::Format("Player %d wins!", game.getWinner());
    }

    dc.SetTextForeground(SUBTITLE_TEXT);
    dc.SetFont(wxFontInfo(11).Bold().FaceName("Avenir Next"));
    dc.DrawText(modeText, statusRect.x + 18, statusRect.y + 12);

    dc.SetTextForeground(TITLE_TEXT);
    dc.SetFont(wxFontInfo(15).Bold().FaceName("Avenir Next"));
    dc.DrawText(statusText, statusRect.x + 18, statusRect.y + 30);

    wxRect outerRect(
        boardRect.x - 18,
        boardRect.y - 18,
        boardRect.width + 36,
        boardRect.height + 36);

    // shadow behind the board
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(SHADOW));
    dc.DrawRoundedRectangle(outerRect.x + 6, outerRect.y + 7, outerRect.width, outerRect.height, 22);

    // main board frame
    dc.SetBrush(wxBrush(BOARD_BODY));
    dc.DrawRoundedRectangle(outerRect.x, outerRect.y, outerRect.width, outerRect.height, 22);

    dc.SetPen(wxPen(BOARD_EDGE, 2));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRoundedRectangle(outerRect.x, outerRect.y, outerRect.width, outerRect.height, 22);

    // accent strip across the top of the board frame
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(CARD_EDGE));
    dc.DrawRoundedRectangle(outerRect.x, outerRect.y, outerRect.width, 6, 22);

    // column labels
    dc.SetTextForeground(TITLE_TEXT);
    dc.SetFont(wxFontInfo(11).Bold().FaceName("Avenir Next"));

    for (int c = 0; c < board.getColumns(); c++)
    {
        wxString label = wxString::Format("%d", c + 1);
        wxCoord tw, th;
        dc.GetTextExtent(label, &tw, &th);

        int labelX = boardRect.x + c * CELL_SIZE + (CELL_SIZE - tw) / 2;
        int labelY = boardRect.y - 28;
        dc.DrawText(label, labelX, labelY);
    }

    // draw each slot and piece
    for (int r = 0; r < board.getRows(); r++)
    {
        for (int c = 0; c < board.getColumns(); c++)
        {
            int x = boardRect.x + c * CELL_SIZE;
            int y = boardRect.y + r * CELL_SIZE;

            int cell = board.getCell(r, c);

            if (cell == 0)
            {
                dc.SetBrush(wxBrush(EMPTY_SLOT));
                dc.SetPen(wxPen(EMPTY_SLOT_EDGE, 2));
            }
            else if (cell == 1)
            {
                dc.SetBrush(wxBrush(PLAYER_ONE));
                dc.SetPen(wxPen(PLAYER_ONE_EDGE, 2));
            }
            else
            {
                dc.SetBrush(wxBrush(PLAYER_TWO));
                dc.SetPen(wxPen(PLAYER_TWO_EDGE, 2));
            }

            dc.DrawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, CELL_SIZE / 2 - 9);
        }
    }

    dc.SetTextForeground(SUBTITLE_TEXT);
    dc.SetFont(wxFontInfo(11).FaceName("Avenir Next"));
    dc.DrawText("Click a column to place your piece", 32, sz.GetHeight() - 34);
}

/**
 * @brief Handles left mouse click events on the panel.
 * Determines whether the click is within the board area, attempts to drop a piece in the selected column, and triggers game-over handling or the AI turn.
 * @param event mouse click event
 */
void ConnectFourPanel::OnLeftClick(wxMouseEvent &event)
{
    if (game.getIsGameOver())
    {
        return;
    }

    if (mode == GameMode::SinglePlayer && game.getCurrentPlayer() == 2)
    {
        return;
    }

    if (aiThinking)
    { // ignore clicks while the AI is in its short "thinking" delay
        return;
    }

    int mouseX = event.GetX();
    int mouseY = event.GetY();

    wxRect boardRect = GetBoardRect();

    if (!boardRect.Contains(mouseX, mouseY))
    {
        return;
    }

    // convert the mouse x-position into a board column
    int column = (mouseX - boardRect.x) / CELL_SIZE;

    if (!game.dropPiece(column))
    {
        soundManager.PlayInvalidMoveSound();
        wxMessageBox("Invalid move.", "Connect Four", wxOK | wxICON_WARNING);
        return;
    }

    soundManager.PlayPlaceTokenSound();
    Refresh();

    if (game.getIsGameOver())
    {
        ShowGameOverDialog();
        return;
    }

    // after the human moves, start a short timer before the AI takes its turn
    if (mode == GameMode::SinglePlayer && game.getCurrentPlayer() == 2)
    {
        aiThinking = true;
        Refresh();
        aiTimer.StartOnce(800); // makes the timer go off just one time after 800 ms
    }
}

/**
 * @brief Handles the AI timer event. Ends the AI thinking delay and triggers the AI's move.
 * @param event timer event
 */
void ConnectFourPanel::OnAITimer(wxTimerEvent &event)
{
    aiThinking = false;
    MakeAIMove();
}

/**
 * @brief Performs the AI's move.
 * Uses the AI object to choose a column, drops the piece, refreshes the display, and checks whether the move ended the game.
 */
void ConnectFourPanel::MakeAIMove()
{
    int aiColumn = ai.chooseColumn(game.getBoard(), 2);

    if (aiColumn == -1)
    {
        return;
    }

    game.dropPiece(aiColumn);
    soundManager.PlayPlaceTokenSound();
    Refresh();

    if (game.getIsGameOver())
    {
        ShowGameOverDialog();
    }
}

/**
 * @brief Records the current game result once after the game ends.
 *
 * The method checks whether statistics have already been recorded for the
 * current finished game. If not, it loads the local statistics file, updates
 * the correct mode section, and marks the game as already counted.
 */
void ConnectFourPanel::RecordStatsIfNeeded()
{
    if (statsRecordedForCurrentGame || !game.getIsGameOver())
    {
        return;
    }

    ConnectFourStatsManager statsManager;
    statsManager.Load(ConnectFourStatsManager::GetDefaultFilePath());

    if (mode == GameMode::SinglePlayer)
    {
        if (ai.getDifficulty() == AIDifficulty::Hard)
        {
            statsManager.RecordHardSinglePlayer(game.getWinner());
        }
        else
        {
            statsManager.RecordNormalSinglePlayer(game.getWinner());
        }
    }
    else
    {
        statsManager.RecordMultiplayer(game.getWinner());
    }

    statsRecordedForCurrentGame = true;
}

void ConnectFourPanel::ShowGameOverDialog() {
    RecordStatsIfNeeded();

    wxString winnerMessage;

    if (game.getWinner() == 0) {
        winnerMessage = "It's a draw!";
        soundManager.PlayDrawSound();
    } else if (mode == GameMode::SinglePlayer) {
        if (game.getWinner() == 1) {
            winnerMessage = "You win!";
            soundManager.PlayWinSound();
        } else {
            winnerMessage = "Computer wins!";
            soundManager.PlayLoseSound();
        }
    } else {
        winnerMessage = wxString::Format("Player %d wins!", game.getWinner());
        soundManager.PlayWinSound();
    }

    wxMessageDialog dialog(
        this,
        winnerMessage + "\n\nWould you like to restart the game or go to the main menu?",
        "Game Over",
        wxYES_NO | wxCANCEL | wxICON_INFORMATION
    );

    dialog.SetYesNoLabels("Restart", "Main Menu");

    int result = dialog.ShowModal();

    if (result == wxID_YES) {
        game.resetGame();
        aiThinking = false;
        statsRecordedForCurrentGame = false;

        if (aiTimer.IsRunning()) {
            aiTimer.Stop();
        }

        Refresh();
    } else if (result == wxID_NO) {
        // Stop any leftover AI delay before returning to the menu
        aiThinking = false;

        if (aiTimer.IsRunning()) {
            aiTimer.Stop();
        }

        // Get the current top-level game window
        wxWindow* topLevel = wxGetTopLevelParent(this);

        // Show the menu as its own dialog, not attached to the current game window
        ConnectFourMenu menu(nullptr);

        int menuResult = menu.ShowModal();

        if (menuResult == wxID_OK) {
            GameMode newMode = menu.getSelectedMode();
            AIDifficulty newDifficulty = menu.getSelectedDifficulty();

            // Create a brand new Connect Four window using the new mode/difficulty
            wxFrame* newFrame = new wxFrame(nullptr, wxID_ANY, "Connect Four",
                                            wxDefaultPosition, wxSize(920, 780));
            newFrame->SetMinSize(wxSize(860, 720));
            newFrame->SetBackgroundColour(wxColour(18, 24, 32));

            new ConnectFourPanel(newFrame, newMode, newDifficulty);
            newFrame->Centre();
            newFrame->Show(true);

            // Close the old game window so it does not stay behind
            if (topLevel != nullptr) {
                topLevel->Destroy();
            }
        } else {
            // If the user cancels from the menu, close the current game window too
            if (topLevel != nullptr) {
                topLevel->Destroy();
            }
        }
    }
}
