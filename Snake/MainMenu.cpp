// MainMenu.cpp
#include "MainMenu.h"
#include <cstdlib>
#include <ctime>

static float frand01_() { return (float)std::rand() / (float)RAND_MAX; }

float MainMenu::EaseOut(float t)
{
    t = std::clamp(t, 0.f, 1.f);
    float u = 1.f - t;
    return 1.f - u * u * u;
}

void MainMenu::SetActions(std::function<void()> startGame,
                          std::function<void()> stats,
                          std::function<void()> tutorials,
                          std::function<void()> exitGame)
{
    actStart_ = std::move(startGame);
    actStats_ = std::move(stats);
    actTutorials_ = std::move(tutorials);
    actExit_ = std::move(exitGame);
}

void MainMenu::SetOptions(std::function<void()> cycleSkin,
                          std::function<void()> cycleDifficulty,
                          std::function<void()> toggleSound)
{
    optCycleSkin_ = std::move(cycleSkin);
    optCycleDifficulty_ = std::move(cycleDifficulty);
    optToggleSound_ = std::move(toggleSound);
}

void MainMenu::SetSfxCallbacks(std::function<void()> onHover,
                               std::function<void()> onClick)
{
    sfxHover_ = std::move(onHover);
    sfxClick_ = std::move(onClick);
}

void MainMenu::Open()
{
    target_ = 1.f;
    selectedIndex_ = 0;
}

void MainMenu::Close()
{
    target_ = 0.f;
    selectedIndex_ = 0;
}

bool MainMenu::IsOpen() const { return target_ > 0.5f; }

float MainMenu::Anim() const { return anim_; }
bool MainMenu::IsInteractive() const { return anim_ > 0.08f; }

void MainMenu::Tick()
{
    anim_ += (target_ - anim_) * 0.18f;
    if (std::fabs(anim_ - target_) < 0.001f)
        anim_ = target_;

    long long now = wxGetUTCTimeMillis().GetValue();
    if (lastMs_ == 0)
        lastMs_ = now;
    double dt = (now - lastMs_) / 1000.0;
    lastMs_ = now;

    if (dt > 0.05)
        dt = 0.05;
    if (dt < 0.0)
        dt = 0.0;

    phase_ += dt * 1.25;

    for (auto &b : buttons_)
    {
        float ht = b.hovered ? 1.f : 0.f;
        b.hoverAnim += (ht - b.hoverAnim) * 0.25f;
        if (std::fabs(b.hoverAnim - ht) < 0.001f)
            b.hoverAnim = ht;

        b.pressAnim += (0.f - b.pressAnim) * 0.35f;
        if (std::fabs(b.pressAnim) < 0.001f)
            b.pressAnim = 0.f;
    }
}

void MainMenu::SetSelected_(int idx)
{
    if (buttons_.empty())
    {
        selectedIndex_ = 0;
        return;
    }
    selectedIndex_ = std::clamp(idx, 0, (int)buttons_.size() - 1);

    for (int i = 0; i < (int)buttons_.size(); ++i)
        buttons_[i].hovered = (i == selectedIndex_);
}

void MainMenu::OnKeyDown(int keyCode)
{
    if (!IsInteractive() || buttons_.empty())
        return;

    switch (keyCode)
    {
    case WXK_UP:
        SetSelected_(selectedIndex_ - 1);
        if (sfxHover_)
            sfxHover_();
        break;
    case WXK_DOWN:
        SetSelected_(selectedIndex_ + 1);
        if (sfxHover_)
            sfxHover_();
        break;
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
    {
        auto &b = buttons_[selectedIndex_];
        if (b.onClick)
        {
            b.pressAnim = 1.f;
            if (sfxClick_)
                sfxClick_();
            b.onClick();
        }
        break;
    }
    default:
        break;
    }
}

void MainMenu::OnMouseMove(const wxPoint &p)
{
    mouse_ = p;
    UpdateHover_();
}

void MainMenu::OnMouseDown(const wxPoint &p)
{
    mouse_ = p;
    if (!IsInteractive())
        return;

    for (int i = 0; i < (int)buttons_.size(); ++i)
    {
        if (AnimatedRectFor_(i).Contains(p))
        {
            buttons_[i].pressed = true;
            buttons_[i].pressAnim = 1.f;
            selectedIndex_ = i;
            if (sfxClick_)
                sfxClick_();
            return;
        }
    }
}

void MainMenu::OnMouseUp(const wxPoint &p)
{
    mouse_ = p;
    if (!IsInteractive())
        return;

    for (int i = 0; i < (int)buttons_.size(); ++i)
    {
        auto &b = buttons_[i];
        if (!b.pressed)
            continue;
        b.pressed = false;

        if (AnimatedRectFor_(i).Contains(p) && b.onClick)
        {
            b.onClick();
            return;
        }
    }
}

void MainMenu::UpdateHover_()
{
    if (!IsInteractive())
    {
        for (auto &b : buttons_)
        {
            b.hovered = false;
            b.hoverAnim = 0.f;
        }
        return;
    }

    int hoveredIndex = -1;
    for (int i = 0; i < (int)buttons_.size(); ++i)
    {
        if (AnimatedRectFor_(i).Contains(mouse_))
        {
            hoveredIndex = i;
            break;
        }
    }

    if (hoveredIndex >= 0)
    {
        for (int i = 0; i < (int)buttons_.size(); ++i)
        {
            bool was = buttons_[i].hovered;
            bool now = (i == hoveredIndex);
            buttons_[i].hovered = now;
            if (sfxHover_ && !was && now)
                sfxHover_();
        }
        selectedIndex_ = hoveredIndex;
    }
    else
    {
        for (int i = 0; i < (int)buttons_.size(); ++i)
            buttons_[i].hovered = (i == selectedIndex_);
    }
}

void MainMenu::RebuildButtons_(const wxRect &screen)
{
    buttons_.clear();

    const int btnW = std::min(520, (int)(screen.width * 0.55));
    const int btnH = 60;
    const int gap = 14;

    const int totalH = 7 * btnH + 6 * gap;
    const int startY = screen.y + (int)(screen.height * 0.62) - totalH / 2;
    const int x = screen.x + (screen.width - btnW) / 2;

    Button start{"Start Game", actStart_};
    Button stats{"Stats", actStats_};
    Button tuto{"Tutorials", actTutorials_};

    Button theme{"Theme", [this]()
                 { if (optCycleSkin_) optCycleSkin_(); }};
    Button diff{"Difficulty", [this]()
                { if (optCycleDifficulty_) optCycleDifficulty_(); }};
    Button sound{"Sound", [this]()
                 { if (optToggleSound_) optToggleSound_(); }};

    Button exit{"Exit", actExit_};

    std::vector<Button> tmp{start, stats, tuto, theme, diff, sound, exit};
    for (int i = 0; i < (int)tmp.size(); ++i)
        tmp[i].rect = wxRect(x, startY + i * (btnH + gap), btnW, btnH);

    buttons_ = std::move(tmp);

    selectedIndex_ = std::clamp(selectedIndex_, 0, (int)buttons_.size() - 1);
    SetSelected_(selectedIndex_);
}

wxRect MainMenu::AnimatedRectFor_(int index) const
{
    if (index < 0 || index >= (int)buttons_.size())
        return wxRect();

    float s = EaseOut(anim_);

    float delay = 0.06f * index;
    float local = (s - delay) / 0.55f;
    local = std::clamp(local, 0.f, 1.f);
    float ease = EaseOut(local);

    int dropY = (int)(-22.0 * (1.0 - ease));
    int fadeX = (int)(-18.0 * (1.0 - ease));

    wxRect r = buttons_[index].rect;
    r.Offset(fadeX, dropY);
    return r;
}

static wxColour Mix_(wxColour a, wxColour b, float t, int alphaOverride = -1)
{
    t = std::clamp(t, 0.f, 1.f);
    int r = (int)std::lround(a.Red() + (b.Red() - a.Red()) * t);
    int g = (int)std::lround(a.Green() + (b.Green() - a.Green()) * t);
    int bl = (int)std::lround(a.Blue() + (b.Blue() - a.Blue()) * t);
    int al = (alphaOverride >= 0) ? alphaOverride
                                  : (int)std::lround(a.Alpha() + (b.Alpha() - a.Alpha()) * t);
    return wxColour(r, g, bl, std::clamp(al, 0, 255));
}

void MainMenu::DrawButton_(wxGraphicsContext *gc, Button &b, const Skin &skin, SkinId skinId, int index)
{
    float t = EaseOut(anim_);
    if (t <= 0.f)
        return;

    const bool lightMode = (skinId == SkinId::Light);

    wxRect r = AnimatedRectFor_(index);

    float h = b.hoverAnim * t;
    float time = (float)phase_;

    int lift = (int)(-8 * h);
    int wobbleX = (int)(std::sin(time * 6.0f + index * 1.1f) * (2.0f * h));
    int wobbleY = (int)(std::cos(time * 5.0f + index * 1.3f) * (1.5f * h));

    float p = b.pressAnim * t;
    int punchY = (int)(-3 * p);
    r.Offset(wobbleX, lift + wobbleY + punchY);

    const int radius = 16;

    wxColour base = lightMode
                        ? wxColour(255, 255, 255, (int)(220 * t))
                        : wxColour(0, 0, 0, (int)(135 * t));

    wxColour outline = lightMode
                           ? wxColour(95, 110, 125, (int)(70 * t))
                           : wxColour(255, 255, 255, (int)(42 * t));

    wxColour accent = skin.snakeHead;
    wxColour accent2 = skin.foodGlow;

    if (h > 0.01f)
    {
        wxRect back = r;
        back.Offset(7, 7);

        wxColour backFill = lightMode
                                ? wxColour(accent.Red(), accent.Green(), accent.Blue(), (int)(55 * h))
                                : Mix_(wxColour(0, 0, 0, 0), accent, 0.55f, (int)(115 * h));

        gc->SetBrush(wxBrush(backFill));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRoundedRectangle(back.x, back.y, back.width, back.height, radius);

        wxRect smear1 = r;
        smear1.Offset(12, 12);
        wxRect smear2 = r;
        smear2.Offset(18, 18);

        wxColour smearC = lightMode
                              ? wxColour(accent2.Red(), accent2.Green(), accent2.Blue(), (int)(34 * h))
                              : Mix_(wxColour(0, 0, 0, 0), accent2, 0.45f, (int)(60 * h));

        wxColour smearC2 = lightMode
                               ? wxColour(accent2.Red(), accent2.Green(), accent2.Blue(), (int)(18 * h))
                               : Mix_(wxColour(0, 0, 0, 0), accent2, 0.35f, (int)(34 * h));

        gc->SetBrush(wxBrush(smearC));
        gc->DrawRoundedRectangle(smear1.x, smear1.y, smear1.width, smear1.height, radius);
        gc->SetBrush(wxBrush(smearC2));
        gc->DrawRoundedRectangle(smear2.x, smear2.y, smear2.width, smear2.height, radius);
    }

    gc->SetBrush(wxBrush(base));
    gc->SetPen(wxPen(outline, 1));
    gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);

    if (h > 0.02f)
    {
        wxColour hi = lightMode
                          ? wxColour(90, 110, 130, (int)(65 * h))
                          : Mix_(wxColour(255, 255, 255, 0), skin.text, 0.5f, (int)(70 * h));

        gc->SetPen(wxPen(hi, 2));
        gc->StrokeLine(r.x + 18, r.y + 14, r.x + r.width - 18, r.y + 14);
    }

    wxFont f = wxFontInfo(16).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    wxColour txt = WithAlpha_(skin.text, (int)(240 * t));
    wxColour shadow = lightMode
                          ? wxColour(255, 255, 255, (int)(180 * t))
                          : wxColour(0, 0, 0, (int)(200 * t));

    wxString label = b.label;
    if (b.label == "Theme" || b.label.StartsWith("Theme"))
        label = "Theme: " + curSkinStr_;
    else if (b.label == "Difficulty" || b.label.StartsWith("Difficulty"))
        label = "Difficulty: " + curDiffStr_;
    else if (b.label == "Sound" || b.label.StartsWith("Sound"))
        label = "Sound: " + curSoundStr_;

    wxDouble tw, th, descent, extlead;
    gc->SetFont(f, txt);
    gc->GetTextExtent(label, &tw, &th, &descent, &extlead);

    double tx = r.x + (r.width - tw) / 2.0;
    double ty = r.y + (r.height - th) / 2.0;

    gc->SetFont(f, shadow);
    gc->DrawText(label, tx + 2, ty + 2);

    if (h > 0.01f)
    {
        wxColour glow = WithAlpha_(skin.foodGlow, lightMode ? (int)(70 * h) : (int)(120 * h));
        gc->SetFont(f, glow);
        gc->DrawText(label, tx - 1, ty - 1);
        gc->DrawText(label, tx + 1, ty + 1);
    }

    gc->SetFont(f, txt);
    gc->DrawText(label, tx, ty);
}

// -------------------- Stars / Persona shards --------------------

void MainMenu::EnsureStars_(const wxRect &screen, bool personaMode)
{
    if (starsInit_ && !stars_.empty())
        return;
    starsInit_ = true;

    static bool seeded = false;
    if (!seeded)
    {
        std::srand((unsigned)std::time(nullptr));
        seeded = true;
    }

    const int baseCount = std::clamp((screen.width * screen.height) / 8000, 140, 340);
    const int count = personaMode ? std::clamp(baseCount + 80, 220, 460) : baseCount;

    stars_.clear();
    stars_.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        Star s;
        s.x = (float)(screen.x + frand01_() * screen.width);
        s.y = (float)(screen.y + frand01_() * screen.height);

        float layer = frand01_();

        if (!personaMode)
        {
            s.speed = 6.0f + layer * 38.0f;
            s.size = 0.8f + layer * 1.8f;
            s.vx = 0.35f;
            s.vy = 1.0f;
        }
        else
        {
            s.speed = 18.0f + layer * 85.0f;
            s.size = 0.9f + layer * 2.4f;
            s.vx = 0.75f;
            s.vy = 1.35f;
        }

        s.tw = frand01_() * 6.2831853f;
        stars_.push_back(s);
    }
}

void MainMenu::TickStars_(const wxRect &screen, float dt, bool personaMode)
{
    EnsureStars_(screen, personaMode);
    if (stars_.empty())
        return;

    for (auto &s : stars_)
    {
        s.x += s.speed * s.vx * dt;
        s.y += s.speed * s.vy * dt;
        s.tw += dt * (0.6f + 1.2f * (s.speed / 90.0f));

        if (s.x > screen.x + screen.width + 12)
            s.x = (float)screen.x - 12;
        if (s.y > screen.y + screen.height + 12)
            s.y = (float)screen.y - 12;
    }
}

void MainMenu::DrawStarfield_(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, float t, bool personaMode)
{
    if (t <= 0.001f)
        return;

    EnsureStars_(screen, personaMode);

    if (!personaMode)
    {
        const bool lightMode = (skin.bg1.Red() > 200 && skin.bg1.Green() > 200 && skin.bg1.Blue() > 200);

        wxGraphicsBrush bg = lightMode
                                 ? gc->CreateRadialGradientBrush(
                                       screen.x + screen.width * 0.35,
                                       screen.y + screen.height * 0.25,
                                       screen.x + screen.width * 0.35,
                                       screen.y + screen.height * 0.25,
                                       std::hypot((double)screen.width, (double)screen.height) * 0.90,
                                       wxColour(255, 255, 255, 255),
                                       wxColour(232, 239, 247, 255))
                                 : gc->CreateRadialGradientBrush(
                                       screen.x + screen.width * 0.35,
                                       screen.y + screen.height * 0.25,
                                       screen.x + screen.width * 0.35,
                                       screen.y + screen.height * 0.25,
                                       std::hypot((double)screen.width, (double)screen.height) * 0.90,
                                       wxColour(0, 0, 0, 255),
                                       wxColour(10, 18, 16, 255));

        gc->SetBrush(bg);
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(screen.x, screen.y, screen.width, screen.height);
    }
    else
    {
        wxGraphicsBrush bg = gc->CreateLinearGradientBrush(
            screen.x, screen.y,
            screen.x, screen.y + screen.height,
            wxColour(0, 0, 0, 255),
            wxColour(30, 0, 8, 255));
        gc->SetBrush(bg);
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(screen.x, screen.y, screen.width, screen.height);
    }

    for (const auto &s : stars_)
    {
        float tw = 0.55f + 0.45f * std::sin(s.tw);

        int a = (!personaMode)
                    ? (int)std::lround((120 + 140 * tw) * t)
                    : (int)std::lround((110 + 155 * tw) * t);
        a = std::clamp(a, 0, 255);

        if (!personaMode)
        {
            bool lightMode = (skin.bg1.Red() > 200 && skin.bg1.Green() > 200 && skin.bg1.Blue() > 200);
            bool venom = (((int)s.speed) % 9 == 0);

            wxColour c = venom
                             ? skin.snakeHead
                             : (lightMode ? wxColour(110, 130, 150) : wxColour(255, 255, 255));

            if (lightMode)
                a = std::clamp((int)std::lround((45 + 70 * tw) * t), 0, 150);

            c.Set(c.Red(), c.Green(), c.Blue(), a);

            gc->SetBrush(wxBrush(c));
            gc->SetPen(*wxTRANSPARENT_PEN);

            double r = s.size;
            gc->DrawEllipse(s.x - r, s.y - r, r * 2, r * 2);
        }
        else
        {
            bool red = (((int)s.speed) % 3 == 0);
            wxColour c = red ? wxColour(255, 0, 55) : wxColour(255, 255, 255);
            c.Set(c.Red(), c.Green(), c.Blue(), a);

            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->SetBrush(wxBrush(c));

            const double len = 6.0 + s.size * 3.0;
            const double thick = 1.4 + s.size * 0.35;

            wxPoint2DDouble p1(s.x, s.y);
            wxPoint2DDouble p2(s.x + len, s.y + len * 1.35);

            double nx = -(p2.m_y - p1.m_y);
            double ny = (p2.m_x - p1.m_x);
            double nlen = std::hypot(nx, ny);
            if (nlen < 1e-6)
                nlen = 1.0;
            nx = nx / nlen * thick;
            ny = ny / nlen * thick;

            wxPoint2DDouble pts[4] = {
                wxPoint2DDouble(p1.m_x + nx, p1.m_y + ny),
                wxPoint2DDouble(p1.m_x - nx, p1.m_y - ny),
                wxPoint2DDouble(p2.m_x - nx, p2.m_y - ny),
                wxPoint2DDouble(p2.m_x + nx, p2.m_y + ny),
            };

            wxGraphicsPath path = gc->CreatePath();
            path.MoveToPoint(pts[0].m_x, pts[0].m_y);
            path.AddLineToPoint(pts[1].m_x, pts[1].m_y);
            path.AddLineToPoint(pts[2].m_x, pts[2].m_y);
            path.AddLineToPoint(pts[3].m_x, pts[3].m_y);
            path.CloseSubpath();
            gc->FillPath(path);
        }
    }

    int endA = personaMode ? 105 : 110;
    wxGraphicsBrush vig = gc->CreateRadialGradientBrush(
        screen.x + screen.width * 0.5,
        screen.y + screen.height * 0.55,
        screen.x + screen.width * 0.5,
        screen.y + screen.height * 0.55,
        std::hypot((double)screen.width, (double)screen.height) * 0.78,
        wxColour(0, 0, 0, 0),
        wxColour(0, 0, 0, (int)(endA * t)));
    gc->SetBrush(vig);
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(screen.x, screen.y, screen.width, screen.height);
}

void MainMenu::DrawVoidSnake_(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, float t, bool personaMode)
{
    if (t <= 0.001f)
        return;

    if (personaMode)
    {
        double time = phase_ * 0.65;
        double ampY = screen.height * 0.06;
        double midY = screen.y + screen.height * 0.70;

        double headX = screen.x + std::fmod(time * (screen.width * 0.55), (double)(screen.width + 520)) - 260.0;

        const int N = 80;
        const double segGap = 11.0;

        wxColour body(255, 255, 255);
        body.Set(body.Red(), body.Green(), body.Blue(), (int)(42 * t));
        wxColour head(255, 0, 55);
        head.Set(head.Red(), head.Green(), head.Blue(), (int)(110 * t));
        wxColour glow(255, 0, 90);
        glow.Set(glow.Red(), glow.Green(), glow.Blue(), (int)(36 * t));

        for (int i = 0; i < N; ++i)
        {
            double u = i / (double)(N - 1);

            double x = headX - i * segGap;
            while (x < screen.x - 320)
                x += (screen.width + 640);

            double y = midY + std::sin((x * 0.014) + time * 2.7) * ampY;

            double r = (5.0 + 10.0 * (1.0 - u));
            if (i == 0)
                r *= 1.20;

            gc->SetBrush(wxBrush(glow));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawEllipse(x - r * 1.7, y - r * 1.2, r * 3.4, r * 2.4);

            wxColour segC = (i == 0) ? head : body;
            gc->SetBrush(wxBrush(segC));
            gc->DrawEllipse(x - r, y - r * 0.75, r * 2.0, r * 1.5);
        }
        return;
    }

    double time = phase_ * 0.55;
    double ampY = screen.height * 0.10;
    double midY = screen.y + screen.height * 0.58;
    double headX = screen.x + std::fmod(time * (screen.width * 0.45), (double)(screen.width + 420)) - 220.0;

    const int N = 110;
    const double segGap = 10.5;

    wxColour body = skin.snake;
    wxColour head = skin.snakeHead;
    wxColour glow = skin.snakeGlow;

    body.Set(body.Red(), body.Green(), body.Blue(), (int)(55 * t));
    head.Set(head.Red(), head.Green(), head.Blue(), (int)(95 * t));
    glow.Set(glow.Red(), glow.Green(), glow.Blue(), (int)(42 * t));

    for (int i = 0; i < N; ++i)
    {
        double u = i / (double)(N - 1);

        double x = headX - i * segGap;
        while (x < screen.x - 260)
            x += (screen.width + 520);

        double y = midY + std::sin((x * 0.012) + time * 2.4) * ampY +
                   std::sin((x * 0.020) + time * 1.7) * (ampY * 0.45);

        double r = (6.0 + 12.0 * (1.0 - u));
        if (i == 0)
            r *= 1.35;

        gc->SetBrush(wxBrush(glow));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawEllipse(x - r * 1.6, y - r * 1.2, r * 3.2, r * 2.4);

        wxColour segC = (i == 0) ? head : body;
        gc->SetBrush(wxBrush(segC));
        gc->DrawEllipse(x - r, y - r * 0.75, r * 2.0, r * 1.5);
    }
}

void MainMenu::DrawTitle2Bit_(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, float t, bool personaMode)
{
    if (t <= 0.001f)
        return;

    wxString title = "VENOM DASH";

    auto snap = [](double v, double grid)
    {
        return std::floor(v / grid + 0.5) * grid;
    };

    wxFont f = wxFontInfo(personaMode ? 58 : 54).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    wxFont sf = wxFontInfo(13).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");

    double breathe = 1.0 + 0.010 * std::sin(phase_ * 2.8);
    double baseYBob = std::sin(phase_ * 1.6) * 0.6;

    double glowGate = std::sin(phase_ * 0.85);
    glowGate = std::max(0.0, glowGate);
    glowGate = glowGate * glowGate * glowGate;

    double waveSpeed = 7.5;
    double waveStep = 0.38;
    double bounceAmp = 7.0;

    wxDouble tw, th, descent, extlead;
    gc->SetFont(f, *wxWHITE);
    gc->GetTextExtent(title, &tw, &th, &descent, &extlead);

    double x0 = screen.x + (screen.width - tw) / 2.0;
    double y0 = screen.y + screen.height * (personaMode ? 0.110 : 0.125) + baseYBob;

    const double grid = 2.0;
    x0 = snap(x0, grid);
    y0 = snap(y0, grid);

    wxColour shadow(0, 0, 0, (int)(235 * t));
    wxColour outline(personaMode ? 0 : 35, personaMode ? 0 : 35, personaMode ? 0 : 35, (int)(245 * t));
    wxColour fill(245, 245, 245, (int)(250 * t));

    wxColour venom = personaMode ? wxColour(255, 0, 55) : skin.snakeHead;
    venom.Set(venom.Red(), venom.Green(), venom.Blue(), (int)std::lround((personaMode ? 140 : 110) * glowGate * t));

    auto drawChar = [&](wxString ch, double x, double y, double bouncePx)
    {
        y -= bouncePx;
        x = snap(x, grid);
        y = snap(y, grid);

        if (venom.Alpha() > 2)
        {
            gc->SetFont(f, venom);
            gc->DrawText(ch, x - 1, y - 1);
            gc->DrawText(ch, x + 1, y + 1);
        }

        gc->SetFont(f, shadow);
        gc->DrawText(ch, x + 4, y + 4);

        gc->SetFont(f, outline);
        gc->DrawText(ch, x - 2, y);
        gc->DrawText(ch, x + 2, y);
        gc->DrawText(ch, x, y - 2);
        gc->DrawText(ch, x, y + 2);
        gc->DrawText(ch, x - 2, y - 2);
        gc->DrawText(ch, x + 2, y - 2);
        gc->DrawText(ch, x - 2, y + 2);
        gc->DrawText(ch, x + 2, y + 2);

        gc->SetFont(f, fill);
        gc->DrawText(ch, x, y);
    };

    double penX = x0;
    for (int i = 0; i < (int)title.length(); ++i)
    {
        wxString ch = title.Mid(i, 1);

        wxDouble cw, chh;
        gc->SetFont(f, fill);
        gc->GetTextExtent(ch, &cw, &chh, &descent, &extlead);

        double w = std::sin(phase_ * waveSpeed - i * waveStep);
        w = std::max(0.0, w);
        w = w * w;
        double bouncePx = bounceAmp * w * t;

        double y = y0 + (1.0 - breathe) * 8.0;
        drawChar(ch, penX, y, bouncePx);
        penX += cw;
    }

    gc->SetFont(sf, wxColour(220, 220, 220, (int)(170 * t)));
    double sy = snap(screen.y + screen.height * (personaMode ? 0.205 : 0.215), grid);

    wxColour line(255, 255, 255, (int)(42 * t));
    gc->SetPen(wxPen(line, 1));
    gc->StrokeLine(screen.x + screen.width * 0.33, sy + 20, screen.x + screen.width * 0.67, sy + 20);
}

void MainMenu::Draw(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
                    SkinId skinId,
                    const wxString &skinStr,
                    const wxString &difficultyStr,
                    const wxString &soundStr)
{
    if (anim_ <= 0.001f)
        return;

    curSkinStr_ = skinStr;
    curDiffStr_ = difficultyStr;
    curSoundStr_ = soundStr;

    const bool personaMode = (skinId == SkinId::AdvancedGrid);

    if (screen != lastLayoutScreen_)
    {
        RebuildButtons_(screen);
        lastLayoutScreen_ = screen;
        starsInit_ = false;
        stars_.clear();
        EnsureStars_(screen, personaMode);
    }

    TickStars_(screen, 1.0f / 60.0f, personaMode);

    float t = EaseOut(anim_);
    const bool lightMode = (skinId == SkinId::Light);

    gc->SetBrush(wxBrush(
        lightMode
            ? wxColour(255, 255, 255, (int)(36 * t))
            : wxColour(0, 0, 0, (int)(32 * t))));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(screen.x, screen.y, screen.width, screen.height);

    DrawStarfield_(gc, screen, skin, t, personaMode);
    DrawVoidSnake_(gc, screen, skin, t, personaMode);
    DrawTitle2Bit_(gc, screen, skin, t, personaMode);

    for (int i = 0; i < (int)buttons_.size(); ++i)
        DrawButton_(gc, buttons_[i], skin, skinId, i);

    wxFont hintF = wxFontInfo(11).Family(wxFONTFAMILY_SWISS).FaceName("Segoe UI");
    gc->SetFont(hintF, WithAlpha_(skin.text, (int)(175 * t)));
    gc->DrawText("Arrow Keys + Enter • Mouse hover floats",
                 screen.x + 18,
                 screen.y + screen.height - 26);
}