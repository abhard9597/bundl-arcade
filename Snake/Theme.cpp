#include "Theme.h"
#include <algorithm>
#include <cmath>

static Skin MakeSkin(wxColour bg1, wxColour bg2, wxColour hudBg, wxColour text,
                     wxColour snake, wxColour head, wxColour food,
                     wxColour foodGlow, wxColour snakeGlow)
{
    Skin s;
    s.bg1 = bg1;
    s.bg2 = bg2;
    s.hudBg = hudBg;
    s.text = text;
    s.snake = snake;
    s.snakeHead = head;
    s.food = food;
    s.foodGlow = foodGlow;
    s.snakeGlow = snakeGlow;
    s.hudFontSmall = wxFontInfo(10).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    s.hudFontBig = wxFontInfo(22).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    return s;
}

Skin GetSkin(SkinId id)
{
    switch (id)
    {
    case SkinId::Classic:
        return MakeSkin({12, 12, 14}, {32, 32, 36}, {0, 0, 0, 140}, {240, 240, 240},
                        {0, 200, 90}, {0, 255, 120}, {240, 80, 80}, {255, 140, 140}, {90, 255, 190});

    case SkinId::Light:
        return MakeSkin({252, 253, 255}, {236, 241, 247}, {255, 255, 255, 235}, {28, 36, 46},
                        {34, 170, 90}, {22, 130, 70}, {220, 88, 88}, {255, 170, 170}, {110, 220, 160});

    case SkinId::Neon:
        return MakeSkin({6, 6, 20}, {20, 10, 60}, {0, 0, 0, 140}, {220, 220, 255},
                        {0, 220, 255}, {160, 255, 255}, {255, 0, 200}, {255, 140, 240}, {140, 255, 255});

    case SkinId::Amber:
        return MakeSkin({18, 8, 0}, {60, 25, 0}, {0, 0, 0, 140}, {255, 240, 210},
                        {255, 170, 0}, {255, 220, 120}, {120, 200, 255}, {170, 230, 255}, {255, 210, 120});

    case SkinId::Mono:
        return MakeSkin({0, 0, 0}, {20, 20, 20}, {0, 0, 0, 160}, {240, 240, 240},
                        {210, 210, 210}, {255, 255, 255}, {160, 160, 160}, {210, 210, 210}, {255, 255, 255});

    case SkinId::CRT:
        return MakeSkin({5, 10, 5}, {0, 0, 0}, {0, 0, 0, 150}, {190, 255, 190},
                        {120, 255, 120}, {220, 255, 220}, {255, 120, 120}, {255, 190, 190}, {190, 255, 190});

    case SkinId::AdvancedGrid:
        return MakeSkin({5, 5, 8},
                        {90, 0, 20},
                        {0, 0, 0, 190},
                        {255, 255, 255},
                        {255, 255, 255},
                        {255, 0, 40},
                        {255, 255, 255},
                        {255, 0, 80},
                        {255, 0, 120});
    }
    return GetSkin(SkinId::Classic);
}
void DrawBackground(wxGraphicsContext *gc, const wxRect &r, const Skin &s)
{
    wxGraphicsGradientStops stops(s.bg1, s.bg2);
    auto brush = gc->CreateLinearGradientBrush(r.x, r.y, r.x, r.y + r.height, stops);
    gc->SetBrush(brush);
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(r.x, r.y, r.width, r.height);
}

void DrawRoundedCell(wxGraphicsContext *gc, const wxRect &r, const wxColour &fill, int radius)
{
    gc->SetBrush(wxBrush(fill));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);
}

void DrawGlowDot(wxGraphicsContext *gc, const wxPoint &c, int rad, const wxColour &core, const wxColour &glow)
{
    wxColour g2 = glow;
    g2.Set(g2.Red(), g2.Green(), g2.Blue(), 70);
    wxColour g1 = glow;
    g1.Set(g1.Red(), g1.Green(), g1.Blue(), 140);

    gc->SetBrush(wxBrush(g2));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawEllipse(c.x - rad * 2, c.y - rad * 2, rad * 4, rad * 4);

    gc->SetBrush(wxBrush(g1));
    gc->DrawEllipse(c.x - rad * 3 / 2, c.y - rad * 3 / 2, rad * 3, rad * 3);

    gc->SetBrush(wxBrush(core));
    gc->DrawEllipse(c.x - rad, c.y - rad, rad * 2, rad * 2);
}

void DrawCrtOverlay(wxGraphicsContext *gc, const wxRect &r)
{
    gc->SetPen(*wxTRANSPARENT_PEN);
    for (int y = r.y; y < r.y + r.height; y += 3)
    {
        wxColour line(0, 0, 0, 25);
        gc->SetBrush(wxBrush(line));
        gc->DrawRectangle(r.x, y, r.width, 1);
    }
}

// minimal gameplay HUD
void DrawHudScoreOnly(wxGraphicsContext *gc, const wxRect &r, const Skin &s,
                      int score, bool showHint, int scorePulseTicks)
{
    if (showHint)
    {
        gc->SetFont(s.hudFontSmall, wxColour(s.text.Red(), s.text.Green(), s.text.Blue(), 190));
        gc->DrawText("P: Pause    R: Restart", r.x + 16, r.y + 10);
    }

    wxString scoreStr;
    scoreStr << score;
    wxColour shadow(0, 0, 0, 180);

    wxDouble tw, th, descent, extlead;
    gc->SetFont(s.hudFontBig, s.text);
    gc->GetTextExtent(scoreStr, &tw, &th, &descent, &extlead);

    double sx = r.x + r.width - 18 - tw;
    double sy = r.y + (r.height - th) / 2.0;

    gc->SetFont(s.hudFontBig, shadow);
    gc->DrawText(scoreStr, sx + 2, sy + 2);

    if (scorePulseTicks > 0)
    {
        int alpha = std::min(180, 60 + scorePulseTicks * 12);
        wxColour glow = s.text;
        glow.Set(glow.Red(), glow.Green(), glow.Blue(), alpha);
        gc->SetFont(s.hudFontBig, glow);
        gc->DrawText(scoreStr, sx - 1, sy - 1);
        gc->DrawText(scoreStr, sx + 1, sy + 1);
    }

    gc->SetFont(s.hudFontBig, s.text);
    gc->DrawText(scoreStr, sx, sy);
}

void DrawAdvancedGrid(wxGraphicsContext *gc, const wxRect &gridRect,
                      int cols, int rows, int cell, int frameCounter,
                      const Skin &s)
{
    if (cols <= 0 || rows <= 0 || cell <= 0)
        return;

    const double t = frameCounter * 0.08; // animation speed

    // Subtle dark overlay inside the grid to separate it from the background
    gc->SetBrush(wxBrush(wxColour(0, 0, 0, 110)));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(gridRect.x, gridRect.y, gridRect.width, gridRect.height);

    // Base colors derived from the skin, Persona-style red and white
    wxColour accentRed = s.foodGlow;
    wxColour white = s.text;

    // Shifting checker blocks. Gives a dynamic, interactive feel
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            const int cellX = gridRect.x + x * cell;
            const int cellY = gridRect.y + y * cell;

            // Diagonal pattern grouping
            int stripe = (x + y) % 4;
            if (stripe != 0 && stripe != 1)
                continue; // leave some cells empty for breathing room

            double phase = t + x * 0.35 + y * 0.55;
            double pulse = 0.5 + 0.5 * std::sin(phase);

            // Small sideways slide to make blocks feel like they’re drifting
            double slide = std::sin(phase * 0.7) * 0.35;
            int offsetX = (int)std::round(slide * cell * 0.4);

            int margin = 2 + (int)std::round(2.0 * (1.0 - pulse));
            wxRect r2(cellX + margin + offsetX,
                      cellY + margin,
                      cell - 2 * margin,
                      cell - 2 * margin);

            if (r2.width <= 0 || r2.height <= 0)
                continue;

            wxColour c = (stripe == 0) ? accentRed : white;

            int baseAlpha = (stripe == 0)
                                ? 90 + (int)std::round(80.0 * pulse)  // red blocks
                                : 30 + (int)std::round(50.0 * pulse); // white overlays

            baseAlpha = std::clamp(baseAlpha, 0, 190);
            c.Set(c.Red(), c.Green(), c.Blue(), baseAlpha);

            gc->SetBrush(wxBrush(c));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRoundedRectangle(r2.x, r2.y, r2.width, r2.height, 3);
        }
    }

    // Grid lines. Thin white, with occasional red lanes that sweep across
    wxColour baseLine = white;
    baseLine.Set(baseLine.Red(), baseLine.Green(), baseLine.Blue(), 40);

    wxColour hotLine = accentRed;
    hotLine.Set(hotLine.Red(), hotLine.Green(), hotLine.Blue(), 90);

    int sweepX = (int)std::floor(t * 1.4);
    int sweepY = (int)std::floor(t * 1.3);

    for (int x = 0; x <= cols; ++x)
    {
        bool isHot = ((x + sweepX) % 6) == 0;
        wxColour c = isHot ? hotLine : baseLine;
        gc->SetPen(wxPen(c, isHot ? 2 : 1));
        int xx = gridRect.x + x * cell;
        gc->StrokeLine(xx, gridRect.y, xx, gridRect.y + gridRect.height);
    }

    for (int y = 0; y <= rows; ++y)
    {
        bool isHot = ((y + sweepY) % 6) == 0;
        wxColour c = isHot ? hotLine : baseLine;
        gc->SetPen(wxPen(c, isHot ? 2 : 1));
        int yy = gridRect.y + y * cell;
        gc->StrokeLine(gridRect.x, yy, gridRect.x + gridRect.width, yy);
    }

    // Bold diagonal slash, softly pulsing, P5-style
    {
        double dpulse = 0.5 + 0.5 * std::sin(t * 0.9);
        int alpha = 40 + (int)std::round(70.0 * dpulse);
        alpha = std::clamp(alpha, 0, 160);

        wxColour slash = accentRed;
        slash.Set(slash.Red(), slash.Green(), slash.Blue(), alpha);

        gc->SetPen(wxPen(slash, 3));
        gc->StrokeLine(gridRect.x - 12,
                       gridRect.y + gridRect.height + 12,
                       gridRect.x + gridRect.width + 12,
                       gridRect.y - 12);
    }
}