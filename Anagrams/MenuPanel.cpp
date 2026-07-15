#include "MenuPanel.h"
#include "AppTheme.h"
#include "App.h"

#include <wx/app.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/graphics.h>
#include <wx/stdpaths.h>
#include <wx/sound.h>

#include "SoundManager.h"

namespace
{
    const AppTheme::Palette THEME = AppTheme::GetPalette();

    const wxColour BG_FALLBACK = THEME.bg;
    const wxColour PANEL_OVERLAY = wxColour(5, 8, 12, 90);
    const wxColour PANEL_FILL = wxColour(THEME.card.Red(), THEME.card.Green(), THEME.card.Blue(), 165);
    const wxColour PANEL_EDGE = THEME.cardEdge;

    const wxColour TILE = THEME.card;
    const wxColour TILE_HOVER = THEME.buttonHover;
    const wxColour TILE_PRESS = THEME.buttonPress;

    const wxColour TITLE_TEXT = THEME.text;
    const wxColour SUBTITLE_TEXT = THEME.subtle;
    const wxColour SHADOW = THEME.shadow;

    static wxBitmap g_cachedBg;
    static bool g_hasBg = false;
    static int g_bgDrawX = 0;
    static int g_bgDrawY = 0;

    inline wxString GetAssetPath(const wxString &relative)
    {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exeFile(exePath);
        return exeFile.GetPath() + "/" + relative;
    }

    inline wxString GetBundlSoundPath(const wxString &fileName)
    {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName exeFile(exePath);
        return exeFile.GetPath() + "/../bundlApp/addFiles/" + fileName;
    }

    inline void PlayBundlHoverSound()
    {
        wxString path = GetBundlSoundPath("buttonHoverSound.wav");

        if (!wxFileExists(path))
            return;

        wxSound sound(path);
        if (sound.IsOk())
            sound.Play(wxSOUND_ASYNC);
    }

    inline void PlayBundlClickSound()
    {
        wxString path = GetBundlSoundPath("buttonClickSound.wav");

        if (!wxFileExists(path))
            return;

        wxSound sound(path);
        if (sound.IsOk())
            sound.Play(wxSOUND_ASYNC);
    }

    inline wxRect GetCenteredPanelRect(const wxSize &size)
    {
        const int panelWidth = 640;
        const int panelHeight = 540;
        int x = (size.GetWidth() - panelWidth) / 2;
        int y = (size.GetHeight() - panelHeight) / 2;
        return wxRect(x, y, panelWidth, panelHeight);
    }

    inline void DrawScaledBackground(wxDC &dc, const wxBitmap &backgroundBitmap, const wxSize &size)
    {
        if (backgroundBitmap.IsOk())
        {
            wxImage bg = backgroundBitmap.ConvertToImage();

            double imgW = static_cast<double>(bg.GetWidth());
            double imgH = static_cast<double>(bg.GetHeight());
            double winW = static_cast<double>(size.GetWidth());
            double winH = static_cast<double>(size.GetHeight());

            double scale = std::max(winW / imgW, winH / imgH);

            int drawW = static_cast<int>(imgW * scale);
            int drawH = static_cast<int>(imgH * scale);

            bg.Rescale(drawW, drawH, wxIMAGE_QUALITY_HIGH);

            int x = (size.GetWidth() - drawW) / 2;
            int y = (size.GetHeight() - drawH) / 2;

            dc.DrawBitmap(wxBitmap(bg), x, y, false);

            g_cachedBg = wxBitmap(bg);
            g_hasBg = true;
            g_bgDrawX = x;
            g_bgDrawY = y;
        }
        else
        {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(BG_FALLBACK));
            dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

            g_cachedBg = wxBitmap();
            g_hasBg = false;
            g_bgDrawX = 0;
            g_bgDrawY = 0;
        }

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(PANEL_OVERLAY));
        dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    }

    class MenuTile : public wxPanel
    {
    public:
        // Constructor for a menu tile with a title and subtitle, and sets up event handlers for hover and click effects.
        MenuTile(wxWindow *parent, const wxString &title, const wxString &subtitle)
            : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
              m_titleText(title),
              m_subtitleText(subtitle)
        {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetDoubleBuffered(true);
            SetCursor(wxCursor(wxCURSOR_HAND));
            SetMinSize(wxSize(520, 78));

            Bind(wxEVT_PAINT, &MenuTile::OnPaint, this);
            Bind(wxEVT_ENTER_WINDOW, &MenuTile::OnEnter, this);
            Bind(wxEVT_LEAVE_WINDOW, &MenuTile::OnLeave, this);
            Bind(wxEVT_LEFT_DOWN, &MenuTile::OnDown, this);
            Bind(wxEVT_LEFT_UP, &MenuTile::OnUp, this);
        }

        void SetOnClick(std::function<void()> fn)
        {
            m_onClick = std::move(fn);
        }

    private:
        void OnEnter(wxMouseEvent &event)
        {
            m_hovered = true;

            if (!m_hoverSoundPlayed)
            {
                PlayBundlHoverSound();
                m_hoverSoundPlayed = true;
            }

            Refresh();
            event.Skip();
        }

        void OnLeave(wxMouseEvent &event)
        {
            m_hovered = false;
            m_hoverSoundPlayed = false;

            if (!m_pressed)
                Refresh();

            event.Skip();
        }

        void OnDown(wxMouseEvent &event)
        {
            m_pressed = true;
            Refresh();

            if (!HasCapture())
                CaptureMouse();

            event.Skip();
        }

        void OnUp(wxMouseEvent &event)
        {
            if (HasCapture())
                ReleaseMouse();

            bool wasPressed = m_pressed;
            m_pressed = false;

            wxPoint p = ScreenToClient(wxGetMousePosition());
            bool inside = GetClientRect().Contains(p);
            m_hovered = inside;
            Refresh();

            if (wasPressed && inside && m_onClick)
            {
                PlayBundlClickSound();
                auto fn = m_onClick;
                wxTheApp->CallAfter([fn]()
                                    { fn(); });
                return;
            }

            event.Skip();
        }

        void DrawBackgroundBehindMe(wxDC &dc)
        {
            wxSize sz = GetClientSize();

            if (g_hasBg && g_cachedBg.IsOk())
            {
                wxPoint pos = GetPosition();

                int srcX = pos.x - g_bgDrawX;
                int srcY = pos.y - g_bgDrawY;

                if (srcX >= 0 &&
                    srcY >= 0 &&
                    srcX + sz.GetWidth() <= g_cachedBg.GetWidth() &&
                    srcY + sz.GetHeight() <= g_cachedBg.GetHeight())
                {
                    wxBitmap sub = g_cachedBg.GetSubBitmap(wxRect(srcX, srcY, sz.GetWidth(), sz.GetHeight()));
                    dc.DrawBitmap(sub, 0, 0, false);
                }
                else
                {
                    dc.SetPen(*wxTRANSPARENT_PEN);
                    dc.SetBrush(wxBrush(BG_FALLBACK));
                    dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
                }
            }
            else
            {
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.SetBrush(wxBrush(BG_FALLBACK));
                dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
            }

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(PANEL_OVERLAY));
            dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());

            dc.SetBrush(wxBrush(PANEL_FILL));
            dc.DrawRectangle(0, 0, sz.GetWidth(), sz.GetHeight());
        }

        void OnPaint(wxPaintEvent &)
        {
            wxAutoBufferedPaintDC baseDc(this);
            wxGCDC dc(baseDc);

            DrawBackgroundBehindMe(dc);

            wxGraphicsContext *gc = dc.GetGraphicsContext();
            if (!gc)
                return;

            wxSize sz = GetClientSize();
            const double w = static_cast<double>(sz.GetWidth());
            const double h = static_cast<double>(sz.GetHeight());
            const double radius = 18.0;

            wxColour body = TILE;
            if (m_pressed)
                body = TILE_PRESS;
            else if (m_hovered)
                body = TILE_HOVER;

            gc->SetPen(*wxTRANSPARENT_PEN);

            gc->SetBrush(wxBrush(SHADOW));
            gc->DrawRoundedRectangle(6.0, 7.0, w - 12.0, h - 12.0, radius);

            gc->SetBrush(wxBrush(body));
            gc->DrawRoundedRectangle(0.0, 0.0, w - 10.0, h - 10.0, radius);

            gc->SetPen(wxPen(PANEL_EDGE, 1));
            gc->SetBrush(*wxTRANSPARENT_BRUSH);
            gc->DrawRoundedRectangle(0.0, 0.0, w - 10.0, h - 10.0, radius);

            gc->SetFont(wxFont(wxFontInfo(16).Family(wxFONTFAMILY_SWISS).Bold()), TITLE_TEXT);
            gc->DrawText(m_titleText, 14.0, 14.0);

            gc->SetFont(wxFont(wxFontInfo(11).Family(wxFONTFAMILY_SWISS)), SUBTITLE_TEXT);
            gc->DrawText(m_subtitleText, 14.0, 40.0);
        }

    private:
        wxString m_titleText;
        wxString m_subtitleText;
        std::function<void()> m_onClick;
        bool m_pressed = false;
        bool m_hovered = false;
        bool m_hoverSoundPlayed = false;
    };
}

MenuPanel::MenuPanel(wxWindow *parent) : wxPanel(parent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);

    // Load the same style of background asset used by the launcher menu.
    wxString bgPath = GetAssetPath("addFiles/menu_bg.png");
    if (wxFileExists(bgPath))
    {
        wxImage img;
        if (img.LoadFile(bgPath))
            m_bgOriginal = wxBitmap(img);
    }

    Bind(wxEVT_PAINT, [this](wxPaintEvent &)
         {
        wxAutoBufferedPaintDC baseDc(this);
        wxGCDC dc(baseDc);

        dc.Clear();

        wxSize sz = GetClientSize();
        DrawScaledBackground(dc, m_bgOriginal, sz);

        wxGraphicsContext *gc = dc.GetGraphicsContext();
        if (!gc)
            return;

        wxRect panelRect = GetCenteredPanelRect(sz);

        gc->SetPen(wxPen(PANEL_EDGE, 1));
        gc->SetBrush(wxBrush(PANEL_FILL));
        gc->DrawRoundedRectangle(panelRect.x, panelRect.y, panelRect.width, panelRect.height, 18.0);

        dc.SetTextForeground(TITLE_TEXT);
        dc.SetFont(wxFont(wxFontInfo(50).Family(wxFONTFAMILY_SWISS).Bold()));

        wxString title = "ANAGRAMS";
        wxSize titleSize = dc.GetTextExtent(title);
        int titleX = panelRect.x + (panelRect.width - titleSize.GetWidth()) / 2;
        int titleY = panelRect.y + 26;
        dc.DrawText(title, titleX, titleY);

        dc.SetTextForeground(SUBTITLE_TEXT);
        dc.SetFont(wxFont(wxFontInfo(14).Family(wxFONTFAMILY_SWISS)));

        wxString subtitle = "Make as many words as you can in 60 seconds.";
        wxSize subtitleSize = dc.GetTextExtent(subtitle);
        int subtitleX = panelRect.x + (panelRect.width - subtitleSize.GetWidth()) / 2;
        int subtitleY = titleY + titleSize.GetHeight() + 8;
        dc.DrawText(subtitle, subtitleX, subtitleY); });

    Bind(wxEVT_SIZE, [this](wxSizeEvent &event)
         {
        Refresh();
        event.Skip(); });

    auto *root = new wxBoxSizer(wxVERTICAL);
    root->AddSpacer(190);

    auto *column = new wxBoxSizer(wxVERTICAL);

    auto *newGameTile = new MenuTile(this, "New Game", "Start a fresh 60-second round");
    auto *tutorialTile = new MenuTile(this, "Tutorial", "Learn the rules and shortcuts");
    auto *statsTile = new MenuTile(this, "View Stats", "See your best scores");
    auto *quitTile = new MenuTile(this, "Quit", "Exit Anagrams");

    column->Add(newGameTile, 0, wxALIGN_CENTER | wxBOTTOM, 14);
    column->Add(tutorialTile, 0, wxALIGN_CENTER | wxBOTTOM, 14);
    column->Add(statsTile, 0, wxALIGN_CENTER | wxBOTTOM, 14);
    column->Add(quitTile, 0, wxALIGN_CENTER);

    newGameTile->SetOnClick([this]
                            { if (m_onNewGame) m_onNewGame(); });
    tutorialTile->SetOnClick([this]
                             { if (m_onTutorial) m_onTutorial(); });
    statsTile->SetOnClick([this]
                          { if (m_onStats) m_onStats(); });
    quitTile->SetOnClick([this]
                         { if (m_onQuit) m_onQuit(); });

    root->Add(column, 0, wxALIGN_CENTER_HORIZONTAL);
    root->AddStretchSpacer(1);

    SetSizer(root);
}