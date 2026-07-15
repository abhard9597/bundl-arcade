// PauseMenu.cpp
#include "PauseMenu.h"
#include <cmath>
#include <algorithm>

static wxColour WithAlpha(wxColour c, int a)
{
    c.Set(c.Red(), c.Green(), c.Blue(), std::clamp(a, 0, 255));
    return c;
}

float PauseMenu::EaseOut(float t)
{
    t = std::clamp(t, 0.f, 1.f);
    float u = 1.f - t;
    return 1.f - u * u * u;
}

void PauseMenu::SetActions(std::function<void()> resume,
                           std::function<void()> restart,
                           std::function<void()> stats,
                           std::function<void()> controls,
                           std::function<void()> tutorial,
                           std::function<void()> mainMenu)
{
    actResume_ = std::move(resume);
    actRestart_ = std::move(restart);
    actStats_ = std::move(stats);
    actControls_ = std::move(controls);
    actTutorial_ = std::move(tutorial);
    actMainMenu_ = std::move(mainMenu);
}

void PauseMenu::SetOptions(std::function<void()> toggleWrap,
                           std::function<void()> cycleDifficulty,
                           std::function<void()> cycleSkin,
                           std::function<void()> toggleSound)
{
    optToggleWrap_ = std::move(toggleWrap);
    optCycleDifficulty_ = std::move(cycleDifficulty);
    optCycleSkin_ = std::move(cycleSkin);
    optToggleSound_ = std::move(toggleSound);
}

void PauseMenu::SetSfxCallbacks(std::function<void()> onHover,
                                std::function<void()> onClick)
{
    sfxHover_ = std::move(onHover);
    sfxClick_ = std::move(onClick);
}

void PauseMenu::Open()
{
    target_ = 1.f;
    page_ = Page::Main;
    scrollOffset_ = scrollTarget_ = 0;
    selectedIndex_ = 0;
}
void PauseMenu::Close()
{
    target_ = 0.f;
    page_ = Page::Main;
    scrollOffset_ = scrollTarget_ = 0;
    selectedIndex_ = 0;
}
bool PauseMenu::IsOpen() const { return target_ > 0.5f; }

float PauseMenu::Anim() const { return anim_; }
bool PauseMenu::IsInteractive() const { return anim_ > 0.08f; }

void PauseMenu::SetPage(Page p)
{
    page_ = p;
    scrollOffset_ = scrollTarget_ = 0;
    selectedIndex_ = 0;
}
PauseMenu::Page PauseMenu::GetPage() const { return page_; }

void PauseMenu::ClampScroll_()
{
    const int visibleH = std::max(0, visibleBottomY_ - visibleTopY_);
    const int contentH = std::max(0, contentBottomY_ - visibleTopY_);
    const int maxScroll = std::max(0, contentH - visibleH);
    scrollTarget_ = std::clamp(scrollTarget_, 0, maxScroll);
    scrollOffset_ = std::clamp(scrollOffset_, 0, maxScroll);
}

void PauseMenu::EnsureVisible_(int idx)
{
    if (idx < 0 || idx >= (int)buttons_.size())
        return;

    wxRect r = buttons_[idx].rect;
    r.Offset(0, -scrollOffset_);

    const int pad = 12;
    const int top = visibleTopY_ + pad;
    const int bot = visibleBottomY_ - pad;

    if (r.GetTop() < top)
        scrollTarget_ -= (top - r.GetTop());
    else if (r.GetBottom() > bot)
        scrollTarget_ += (r.GetBottom() - bot);

    ClampScroll_();
}

void PauseMenu::SetSelected_(int idx)
{
    if (buttons_.empty())
    {
        selectedIndex_ = 0;
        return;
    }
    selectedIndex_ = std::clamp(idx, 0, (int)buttons_.size() - 1);
    for (int i = 0; i < (int)buttons_.size(); ++i)
        buttons_[i].hovered = (i == selectedIndex_);
    EnsureVisible_(selectedIndex_);
}

void PauseMenu::Tick()
{
    anim_ += (target_ - anim_) * 0.18f;
    if (std::fabs(anim_ - target_) < 0.001f)
        anim_ = target_;

    scrollOffset_ += (int)std::lround((scrollTarget_ - scrollOffset_) * 0.25);
    if (std::abs(scrollTarget_ - scrollOffset_) <= 1)
        scrollOffset_ = scrollTarget_;

    long long now = wxGetUTCTimeMillis().GetValue();
    if (lastAnimMs_ == 0)
        lastAnimMs_ = now;
    double dt = (now - lastAnimMs_) / 1000.0;
    lastAnimMs_ = now;

    dt = std::clamp(dt, 0.0, 0.05);
    stripPhase_ += dt * 1.20;

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

void PauseMenu::OnKeyDown(int keyCode)
{
    if (!IsInteractive() || buttons_.empty())
        return;

    switch (keyCode)
    {
    case WXK_UP:
        SetSelected_(selectedIndex_ - 1);
        if (page_ == Page::Options && sfxHover_)
            sfxHover_();
        break;
    case WXK_DOWN:
        SetSelected_(selectedIndex_ + 1);
        if (page_ == Page::Options && sfxHover_)
            sfxHover_();
        break;
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
    {
        auto &b = buttons_[selectedIndex_];
        if (b.onClick)
        {
            b.pressAnim = 1.f;
            if (page_ == Page::Options && sfxClick_)
                sfxClick_();
            b.onClick();
        }
        break;
    }
    case WXK_ESCAPE:
        if (page_ != Page::Main)
            SetPage(Page::Main);
        else if (actResume_)
            actResume_();
        break;
    default:
        break;
    }
}

void PauseMenu::OnMouseMove(const wxPoint &p)
{
    mouse_ = p;
    UpdateHover();
}

void PauseMenu::OnMouseDown(const wxPoint &p)
{
    mouse_ = p;
    if (!IsInteractive())
        return;

    const bool clickSfxEnabled = (page_ == Page::Options && (bool)sfxClick_);

    for (int i = 0; i < (int)buttons_.size(); ++i)
    {
        if (AnimatedRectFor(i).Contains(p))
        {
            buttons_[i].pressed = true;
            buttons_[i].pressAnim = 1.f;
            selectedIndex_ = i;
            EnsureVisible_(selectedIndex_);
            if (clickSfxEnabled)
                sfxClick_();
            return;
        }
    }
}

void PauseMenu::OnMouseUp(const wxPoint &p)
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
        if (AnimatedRectFor(i).Contains(p) && b.onClick)
        {
            b.onClick();
            return;
        }
    }
}

void PauseMenu::UpdateHover()
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

    const bool hoverSfxEnabled = (page_ == Page::Options && (bool)sfxHover_);

    int hoveredIndex = -1;
    for (int i = 0; i < (int)buttons_.size(); ++i)
    {
        if (AnimatedRectFor(i).Contains(mouse_))
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
            if (hoverSfxEnabled && !was && now)
                sfxHover_();
        }
        selectedIndex_ = hoveredIndex;
        EnsureVisible_(selectedIndex_);
    }
    else
    {
        for (int i = 0; i < (int)buttons_.size(); ++i)
            buttons_[i].hovered = (i == selectedIndex_);
    }
}

wxRect PauseMenu::AnimatedRectFor(int index) const
{
    if (index < 0 || index >= (int)buttons_.size())
        return wxRect();

    float s = EaseOut(anim_);
    int globalX = (int)(-360.0 * (1.0 - s));

    float delay = 0.06f * index;
    float local = (s - delay) / 0.45f;
    local = std::clamp(local, 0.f, 1.f);
    float ease = EaseOut(local);

    int extraX = (int)(-80.0 * (1.0 - ease));
    int extraY = (int)(8.0 * (1.0 - ease));

    wxRect r = buttons_[index].rect;
    r.Offset(0, -scrollOffset_);
    r.Offset(globalX + extraX, extraY);
    return r;
}

void PauseMenu::RebuildButtons(const wxRect &screen)
{
    buttons_.clear();

    const int bx = screen.x + 60;
    const int by = screen.y + 240;

    const int btnW = std::min(560, (int)(screen.width * 0.55));
    const int btnH = 56;
    const int gap = 14;

    if (page_ == Page::Main)
    {
        std::vector<Button> tmp{
            {"Resume", actResume_},
            {"Restart", actRestart_},
            {"Stats", actStats_},
            {"Additional Options", [this]()
             { this->SetPage(Page::Options); }},
            {"Controls", actControls_},
            {"Tutorial", actTutorial_},
            {"Main Menu", actMainMenu_},
        };
        for (size_t i = 0; i < tmp.size(); ++i)
            tmp[i].rect = wxRect(bx, by + (int)i * (btnH + gap), btnW, btnH);
        buttons_ = std::move(tmp);
    }
    else if (page_ == Page::Options)
    {
        std::vector<Button> tmp{
            {"Wrap Walls", [this]()
             { if (optToggleWrap_) optToggleWrap_(); }},
            {"Difficulty", [this]()
             { if (optCycleDifficulty_) optCycleDifficulty_(); }},
            {"Skin", [this]()
             { if (optCycleSkin_) optCycleSkin_(); }},
            {"Sound", [this]()
             { if (optToggleSound_) optToggleSound_(); }},
            {"Back", [this]()
             { this->SetPage(Page::Main); }},
        };
        for (size_t i = 0; i < tmp.size(); ++i)
            tmp[i].rect = wxRect(bx, by + (int)i * (btnH + gap), btnW, btnH);
        buttons_ = std::move(tmp);
    }
    else
    {
        Button back{"Back", [this]()
                    { this->SetPage(Page::Main); }};
        back.rect = wxRect(bx, by, btnW, btnH);
        buttons_.push_back(back);
    }

    visibleTopY_ = by;
    visibleBottomY_ = screen.y + screen.height - 40;

    contentBottomY_ = visibleTopY_;
    for (const auto &b : buttons_)
        contentBottomY_ = std::max(contentBottomY_, b.rect.GetBottom());

    ClampScroll_();
    selectedIndex_ = 0;
    SetSelected_(0);
}

void PauseMenu::DrawDim(wxGraphicsContext *gc, const wxRect &screen, float a, bool lightMode)
{
    int alpha = (int)(170 * a);
    gc->SetBrush(wxBrush(
        lightMode
            ? wxColour(255, 255, 255, alpha)
            : wxColour(0, 0, 0, alpha)));
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(screen.x, screen.y, screen.width, screen.height);
}

void PauseMenu::DrawSnakeStrip(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, int y, bool leftToRight)
{
    const double cell = 18.0;
    const int count = (int)(screen.width / cell) + 6;

    const double phase = stripPhase_ * (leftToRight ? 1.0 : -1.0);
    double offset = std::fmod(phase * 60.0, cell);
    if (offset < 0)
        offset += cell;

    wxColour seg = WithAlpha(skin.snake, 80);
    wxColour head = WithAlpha(skin.snakeHead, 120);

    for (int i = -3; i < count; ++i)
    {
        const double x = screen.x + i * cell + offset;
        const double wobble = std::sin((i * 0.6) + stripPhase_) * 3.0;
        const double w = cell - 6.0;
        const double h = 10.0;

        gc->SetBrush(wxBrush((i % 12 == 0) ? head : seg));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRoundedRectangle(x, y + wobble, w, h, 6.0);
    }
}

static wxColour Mix(wxColour a, wxColour b, float t, int alphaOverride = -1)
{
    t = std::clamp(t, 0.f, 1.f);
    int r = (int)std::lround(a.Red() + (b.Red() - a.Red()) * t);
    int g = (int)std::lround(a.Green() + (b.Green() - a.Green()) * t);
    int bl = (int)std::lround(a.Blue() + (b.Blue() - a.Blue()) * t);
    int al = (alphaOverride >= 0) ? alphaOverride
                                  : (int)std::lround(a.Alpha() + (b.Alpha() - a.Alpha()) * t);
    return wxColour(r, g, bl, std::clamp(al, 0, 255));
}

void PauseMenu::DrawButton(wxGraphicsContext *gc, Button &b, const Skin &skin, int index, bool lightMode)
{
    float t = EaseOut(anim_);
    if (t <= 0.f)
        return;

    wxRect r = AnimatedRectFor(index);

    float h = b.hoverAnim * t;
    float time = (float)stripPhase_;
    int lift = (int)(-7 * h);
    int wobbleX = (int)(std::sin(time * 6.0f + index * 1.3f) * (2.0f * h));
    int wobbleY = (int)(std::cos(time * 5.0f + index * 1.1f) * (1.0f * h));

    float p = b.pressAnim * t;
    int punchX = (int)(10 * p);
    int punchY = (int)(-3 * p);
    r.Offset(wobbleX + punchX, lift + wobbleY + punchY);

    const int radius = 14;

    wxColour base = lightMode
                        ? wxColour(255, 255, 255, (int)(220 * t))
                        : wxColour(0, 0, 0, (int)(120 * t));

    wxColour outline = lightMode
                           ? wxColour(95, 110, 125, (int)(70 * t))
                           : wxColour(255, 255, 255, (int)(35 * t));

    wxColour accent = skin.snakeHead;
    wxColour accent2 = skin.foodGlow;

    if (h > 0.01f)
    {
        wxRect back = r;
        back.Offset(6, 6);

        wxColour backFill = lightMode
                                ? wxColour(accent.Red(), accent.Green(), accent.Blue(), (int)(55 * h))
                                : Mix(wxColour(0, 0, 0, 0), accent, 0.55f, (int)(110 * h));

        gc->SetBrush(wxBrush(backFill));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRoundedRectangle(back.x, back.y, back.width, back.height, radius);
    }

    gc->SetBrush(wxBrush(base));
    gc->SetPen(wxPen(outline, 1));
    gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, radius);

    wxFont f = wxFontInfo(14).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");

    wxColour normalText = wxColour(
        skin.text.Red(),
        skin.text.Green(),
        skin.text.Blue(),
        (int)(240 * t));

    wxColour hoverText = Mix(normalText, accent2, 0.55f * h, normalText.Alpha());

    wxColour sh = lightMode
                      ? wxColour(255, 255, 255, (int)(160 * t))
                      : wxColour(0, 0, 0, (int)(160 * t));

    gc->SetFont(f, sh);
    gc->DrawText(b.label, r.x + 18 + 1, r.y + 16 + 1);

    gc->SetFont(f, hoverText);
    gc->DrawText(b.label, r.x + 18, r.y + 16);
}

void PauseMenu::DrawMain(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
                         int score,
                         const wxString & /*playerName*/,
                         const wxString & /*typeStr*/,
                         const wxString & /*skinStr*/,
                         const wxString & /*diffStr*/,
                         const wxString & /*wrapStr*/,
                         const wxString & /*ttlStr*/,
                         bool lightMode)
{
    float t = EaseOut(anim_);
    if (t <= 0.f)
        return;

    int x = screen.x + 60 + (int)(-360.0 * (1.0 - t));
    int y = screen.y + 90;

    wxFont titleF = wxFontInfo(22).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    gc->SetFont(titleF, WithAlpha(skin.text, (int)(255 * t)));
    gc->DrawText("PAUSED", x, y);

    for (int i = 0; i < (int)buttons_.size(); ++i)
        DrawButton(gc, buttons_[i], skin, i, lightMode);

    wxFont hintF = wxFontInfo(11).Family(wxFONTFAMILY_SWISS).FaceName("Segoe UI");
    gc->SetFont(hintF, WithAlpha(skin.text, (int)(190 * t)));
    gc->DrawText("Arrow Keys + Enter • Esc backs out", x, y + 44);
}

void PauseMenu::DrawOptions(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
                            const wxString &skinStr,
                            const wxString &diffStr,
                            const wxString &wrapStr,
                            const wxString &soundStr,
                            bool lightMode)
{
    float t = EaseOut(anim_);
    if (t <= 0.f)
        return;

    int x = screen.x + 60 + (int)(-360.0 * (1.0 - t));
    int y = screen.y + 90;

    wxFont titleF = wxFontInfo(18).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    gc->SetFont(titleF, WithAlpha(skin.text, (int)(255 * t)));
    gc->DrawText("OPTIONS", x, y);

    if (buttons_.size() >= 5)
    {
        buttons_[0].label = wxString::Format("Wrap Walls: %s", wrapStr.Contains("ON") ? "ON" : "OFF");
        buttons_[1].label = wxString::Format("Difficulty: %s", diffStr);
        buttons_[2].label = wxString::Format("Skin: %s", skinStr);
        buttons_[3].label = wxString::Format("Sound: %s", soundStr);
        buttons_[4].label = "Back";
    }

    for (int i = 0; i < (int)buttons_.size(); ++i)
        DrawButton(gc, buttons_[i], skin, i, lightMode);
}

void PauseMenu::DrawTutorial(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin, bool lightMode)
{
    float t = EaseOut(anim_);
    if (t <= 0.f)
        return;

    int x = screen.x + 60 + (int)(-360.0 * (1.0 - t));
    int y = screen.y + 90;

    wxFont titleF = wxFontInfo(18).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
    gc->SetFont(titleF, WithAlpha(skin.text, (int)(255 * t)));
    gc->DrawText("TUTORIAL", x, y);

    wxFont bodyF = wxFontInfo(12).Family(wxFONTFAMILY_SWISS).FaceName("Segoe UI");
    gc->SetFont(bodyF, WithAlpha(skin.text, (int)(235 * t)));

    wxString body =
        "Goal:\n"
        "  Eat food to grow and increase score.\n\n"
        "Controls:\n"
        "  Arrow Keys  - Move\n"
        "  P           - Pause/Resume\n"
        "  R           - Restart\n"
        "  Enter       - Restart on Game Over\n";
    gc->DrawText(body, x, y + 44);

    for (int i = 0; i < (int)buttons_.size(); ++i)
        DrawButton(gc, buttons_[i], skin, i, lightMode);
}

void PauseMenu::Draw(wxGraphicsContext *gc, const wxRect &screen, const Skin &skin,
                     int score,
                     const wxString &playerName,
                     const wxString &typeStr,
                     const wxString &skinStr,
                     const wxString &diffStr,
                     const wxString &wrapStr,
                     const wxString &ttlStr,
                     const wxString &soundStr)
{
    if (anim_ <= 0.001f)
        return;

    if (screen != lastLayoutScreen_ || page_ != lastLayoutPage_)
    {
        RebuildButtons(screen);
        lastLayoutScreen_ = screen;
        lastLayoutPage_ = page_;
    }

    float t = EaseOut(anim_);
    const bool lightMode = (skin.bg1.Red() > 200 && skin.bg1.Green() > 200 && skin.bg1.Blue() > 200);

    DrawDim(gc, screen, t, lightMode);
    DrawSnakeStrip(gc, screen, skin, screen.y + 10, true);
    DrawSnakeStrip(gc, screen, skin, screen.y + screen.height - 22, false);

    if (page_ == Page::Main)
    {
        DrawMain(gc, screen, skin, score, playerName, typeStr, skinStr, diffStr, wrapStr, ttlStr, lightMode);
    }
    else if (page_ == Page::Options)
    {
        DrawOptions(gc, screen, skin, skinStr, diffStr, wrapStr, soundStr, lightMode);
    }
    else
    {
        DrawTutorial(gc, screen, skin, lightMode);
    }
}
