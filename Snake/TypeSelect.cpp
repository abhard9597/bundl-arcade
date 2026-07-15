#include "TypeSelect.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <algorithm>
#include <vector>
#include <cmath>

class TypeSelectDialog final : public wxDialog {
public:
    TypeSelectDialog(wxWindow* parent, const Skin& skin, SnakeType initial)
    : wxDialog(parent, wxID_ANY, "Choose Your Type",
               wxDefaultPosition, wxSize(760, 460),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxWANTS_CHARS),
      skin_(skin),
      selection_(initial),
      selectedIndex_(0),
      timer_(this)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);

        cards_.resize(4);
        cards_[0].type = SnakeType::Classic;
        cards_[1].type = SnakeType::Speedy;
        cards_[2].type = SnakeType::Ghost;
        cards_[3].type = SnakeType::Tank;

        // Start selection highlight on the initial type
        for (int i = 0; i < (int)cards_.size(); ++i) {
            if (cards_[i].type == selection_) { selectedIndex_ = i; break; }
        }

        Bind(wxEVT_PAINT,      &TypeSelectDialog::OnPaint_, this);
        Bind(wxEVT_SIZE,       &TypeSelectDialog::OnSize_, this);
        Bind(wxEVT_MOTION,     &TypeSelectDialog::OnMouseMove_, this);
        Bind(wxEVT_LEFT_DOWN,  &TypeSelectDialog::OnMouseDown_, this);
        Bind(wxEVT_LEFT_UP,    &TypeSelectDialog::OnMouseUp_, this);
        Bind(wxEVT_KEY_DOWN,   &TypeSelectDialog::OnKeyDown_, this);
        Bind(wxEVT_TIMER,      &TypeSelectDialog::OnTimer_, this);

        LayoutCards_();
        timer_.Start(16);

        CallAfter([this]{
            this->SetFocus();
            this->SetFocusFromKbd();
        });
    }

    SnakeType GetSelection() const { return selection_; }

private:
    struct Card {
        SnakeType type = SnakeType::Classic;
        wxRect rect;
        bool hovered = false;
        bool pressed = false;
    };

    Skin skin_;
    SnakeType selection_;
    int selectedIndex_ = 0;

    std::vector<Card> cards_;
    wxPoint mouse_{0,0};
    double phase_ = 0.0;
    wxTimer timer_;

    static wxColour WithAlpha(wxColour c, int a) {
        c.Set(c.Red(), c.Green(), c.Blue(), std::clamp(a, 0, 255));
        return c;
    }

    void LayoutCards_() {
        wxSize sz = GetClientSize();
        wxRect r(0,0,sz.x,sz.y);

        const int pad = 22;
        const int gap = 18;

        int w = (r.width  - pad*2 - gap) / 2;
        int h = (r.height - pad*2 - gap) / 2;

        cards_[0].rect = wxRect(r.x + pad,           r.y + pad,           w, h);
        cards_[1].rect = wxRect(r.x + pad + w + gap, r.y + pad,           w, h);
        cards_[2].rect = wxRect(r.x + pad,           r.y + pad + h + gap, w, h);
        cards_[3].rect = wxRect(r.x + pad + w + gap, r.y + pad + h + gap, w, h);
    }

    void UpdateHover_() {
        for (auto& c : cards_) c.hovered = c.rect.Contains(mouse_);
        // If mouse is over a card, move keyboard selection to it
        for (int i = 0; i < (int)cards_.size(); ++i) {
            if (cards_[i].hovered) { selectedIndex_ = i; break; }
        }
    }

    static void DrawWrappedText_(wxGraphicsContext* gc,
                                const wxString& text,
                                const wxFont& font,
                                const wxColour& col,
                                double x, double y,
                                double maxW,
                                double lineH,
                                int maxLines)
    {
        gc->SetFont(font, col);

        wxArrayString words = wxSplit(text, ' ');
        wxString line;
        int lines = 0;

        auto flush = [&](){
            if (line.IsEmpty()) return;
            gc->DrawText(line, x, y + lines * lineH);
            ++lines;
            line.clear();
        };

        for (size_t i = 0; i < words.size(); ++i) {
            wxString next = line.IsEmpty() ? words[i] : (line + " " + words[i]);

            wxDouble tw, th, d, e;
            gc->GetTextExtent(next, &tw, &th, &d, &e);

            if (tw <= maxW) {
                line = next;
            } else {
                flush();
                if (lines >= maxLines) break;
                line = words[i];
            }
        }
        if (lines < maxLines) flush();

        // Add ellipsis if we ran out of lines and still have words left
        if (lines >= maxLines && words.size() > 0) {
            // Overdraw last line with ellipsis if needed
            // (simple approach: just draw "..." at end area)
            gc->DrawText("…", x + maxW - 10, y + (maxLines - 1) * lineH);
        }
    }

    void DrawSnakeIcon_(wxGraphicsContext* gc, const wxRect& r, SnakeType t, float a) {
        const double time = phase_;
        const double cx = r.GetX() + r.GetWidth() * 0.50;
        const double cy = r.GetY() + r.GetHeight() * 0.58;

        wxColour main = skin_.snakeHead;
        wxColour glow = skin_.foodGlow;

        if (t == SnakeType::Ghost) {
            main = WithAlpha(wxColour(220, 240, 255), (int)(150 * a));
            glow = WithAlpha(wxColour(180, 220, 255), (int)(85 * a));
        } else if (t == SnakeType::Tank) {
            main = WithAlpha(wxColour(255, 255, 255), (int)(210 * a));
            glow = WithAlpha(skin_.snakeHead, (int)(95 * a));
        } else if (t == SnakeType::Speedy) {
            main = WithAlpha(skin_.foodGlow, (int)(210 * a));
            glow = WithAlpha(skin_.foodGlow, (int)(110 * a));
        } else {
            main = WithAlpha(main, (int)(210 * a));
            glow = WithAlpha(glow, (int)(95 * a));
        }

        const int segs = 28;
        const double len = r.GetWidth() * 0.80;
        const double amp = r.GetHeight() * 0.11;

        wxGraphicsPath path = gc->CreatePath();
        for (int i = 0; i < segs; ++i) {
            double u = i / (double)(segs - 1);
            double x = (cx - len/2.0) + u * len;

            double speed = (t == SnakeType::Speedy) ? 3.8 : 1.7;
            double wob   = (t == SnakeType::Tank)   ? 0.65 : 1.0;
            double y = cy + std::sin((u * 6.2831853 * wob) + time * speed) * amp;

            if (t == SnakeType::Ghost) {
                y += std::sin(time * 1.35 + u * 4.2) * (amp * 0.50);
            }

            if (i == 0) path.MoveToPoint(x, y);
            else        path.AddLineToPoint(x, y);
        }

        // Glow stroke
        gc->SetPen(wxPen(glow, (t == SnakeType::Tank) ? 10 : 7));
        gc->StrokePath(path);

        // Main stroke
        if (t == SnakeType::Ghost) {
            gc->SetPen(wxPen(main, 4, wxPENSTYLE_SHORT_DASH));
        } else if (t == SnakeType::Tank) {
            gc->SetPen(wxPen(main, 6));
        } else {
            gc->SetPen(wxPen(main, 5));
        }
        gc->StrokePath(path);

        // Head dot
        wxPoint2DDouble end = path.GetCurrentPoint();
        gc->SetBrush(wxBrush(main));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawEllipse(end.m_x - 6, end.m_y - 6, 12, 12);
    }

    void DrawCard_(wxGraphicsContext* gc, Card& c, int idx) {
        SnakeTypeInfo info = GetSnakeTypeInfo(c.type);

        bool kbdSelected = (idx == selectedIndex_);
        wxColour base    = wxColour(0,0,0, 180);
        wxColour outline = wxColour(255,255,255, (c.hovered || kbdSelected) ? 110 : 45);

        wxColour accent = skin_.snakeHead;
        if (c.type == SnakeType::Speedy) accent = skin_.foodGlow;
        if (c.type == SnakeType::Ghost)  accent = wxColour(180, 220, 255);
        if (c.type == SnakeType::Tank)   accent = wxColour(255, 255, 255);

        wxRect r = c.rect;
        if (c.pressed) r.Offset(0, 2);

        // back accent
        if (c.hovered || kbdSelected) {
            wxRect back = r; back.Offset(6, 6);
            wxColour bf = WithAlpha(accent, 65);
            gc->SetBrush(wxBrush(bf));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRoundedRectangle(back.x, back.y, back.width, back.height, 18);
        }

        gc->SetBrush(wxBrush(base));
        gc->SetPen(wxPen(outline, 1));
        gc->DrawRoundedRectangle(r.x, r.y, r.width, r.height, 18);

        // icon
        wxRect iconR(r.x + 18, r.y + 18, r.width - 36, (int)(r.height * 0.44));
        DrawSnakeIcon_(gc, iconR, c.type, 1.0f);

        // text
        wxFont titleF = wxFontInfo(18).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
        wxFont descF  = wxFontInfo(11).Family(wxFONTFAMILY_SWISS).FaceName("Segoe UI");

        wxString name = wxString(SnakeTypeName(c.type));
        wxString desc = wxString(info.desc);

        wxColour txt = WithAlpha(skin_.text, 235);
        wxColour sh(0,0,0,180);

        double tx = r.x + 18;
        double ty = r.y + r.height * 0.58;

        gc->SetFont(titleF, sh);  gc->DrawText(name, tx + 1, ty + 1);
        gc->SetFont(titleF, txt); gc->DrawText(name, tx, ty);

        // wrapped desc (prevents bleeding off)
        DrawWrappedText_(gc, desc, descF, WithAlpha(skin_.text, 205),
                         tx, ty + 28, r.width - 36, 14.0, 3);

        // selected badge
        if (c.type == selection_) {
            wxString tag = "SELECTED";
            wxFont tagF = wxFontInfo(10).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
            wxDouble tw, th, d, e;
            gc->SetFont(tagF, wxColour(0,0,0));
            gc->GetTextExtent(tag, &tw, &th, &d, &e);

            wxRect badge(r.x + r.width - (int)tw - 26, r.y + 14, (int)tw + 16, 18);
            gc->SetBrush(wxBrush(WithAlpha(accent, 180)));
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->DrawRoundedRectangle(badge.x, badge.y, badge.width, badge.height, 8);

            gc->SetFont(tagF, wxColour(0,0,0));
            gc->DrawText(tag, badge.x + 8, badge.y + 2);
        }

        // keyboard focus hint
        if (kbdSelected && !c.hovered) {
            gc->SetPen(wxPen(WithAlpha(accent, 120), 2));
            gc->SetBrush(*wxTRANSPARENT_BRUSH);
            gc->DrawRoundedRectangle(r.x + 2, r.y + 2, r.width - 4, r.height - 4, 16);
        }
    }

    void SelectCurrent_() {
        selection_ = cards_[selectedIndex_].type;
        EndModal(wxID_OK);
    }

    void MoveSelection_(int dx, int dy) {
        // 2x2 grid: index = row*2 + col
        int row = selectedIndex_ / 2;
        int col = selectedIndex_ % 2;
        row = std::clamp(row + dy, 0, 1);
        col = std::clamp(col + dx, 0, 1);
        selectedIndex_ = row * 2 + col;
        selection_ = cards_[selectedIndex_].type; // live preview
        Refresh(false);
    }

    void OnTimer_(wxTimerEvent&) {
        phase_ += 0.016;
        Refresh(false);
    }

    void OnSize_(wxSizeEvent&) {
        LayoutCards_();
        Refresh(false);
    }

    void OnPaint_(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.Clear();
        auto* gc = wxGraphicsContext::Create(dc);
        if (!gc) return;

        wxSize sz = GetClientSize();

        // background
        wxGraphicsGradientStops stops(WithAlpha(wxColour(0,0,0), 235),
                                      WithAlpha(wxColour(20,20,25), 235));
        auto bg = gc->CreateLinearGradientBrush(0,0,0,sz.y, stops);
        gc->SetBrush(bg);
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawRectangle(0,0,sz.x,sz.y);

        // title
        wxFont tF = wxFontInfo(16).Family(wxFONTFAMILY_SWISS).Bold().FaceName("Segoe UI");
        gc->SetFont(tF, WithAlpha(skin_.text, 220));
        gc->DrawText("Choose your snake type", 18, 10);

        for (int i = 0; i < (int)cards_.size(); ++i) DrawCard_(gc, cards_[i], i);

        // footer hint
        wxFont f = wxFontInfo(11).Family(wxFONTFAMILY_SWISS).FaceName("Segoe UI");
        gc->SetFont(f, WithAlpha(skin_.text, 190));
        gc->DrawText("Arrow keys to select • Enter to confirm • Esc to cancel • Click a card to confirm",
                     18, sz.y - 28);

        delete gc;
    }

    void OnMouseMove_(wxMouseEvent& e) {
        mouse_ = e.GetPosition();
        UpdateHover_();
        Refresh(false);
    }

    void OnMouseDown_(wxMouseEvent& e) {
        mouse_ = e.GetPosition();
        UpdateHover_();
        for (auto& c : cards_) {
            if (c.rect.Contains(mouse_)) c.pressed = true;
        }
        Refresh(false);
    }

    void OnMouseUp_(wxMouseEvent& e) {
        mouse_ = e.GetPosition();
        UpdateHover_();
        for (int i = 0; i < (int)cards_.size(); ++i) {
            auto& c = cards_[i];
            bool wasPressed = c.pressed;
            c.pressed = false;
            if (wasPressed && c.rect.Contains(mouse_)) {
                selectedIndex_ = i;
                SelectCurrent_();
                return;
            }
        }
        Refresh(false);
    }

    void OnKeyDown_(wxKeyEvent& e) {
        int k = e.GetKeyCode();
        switch (k) {
            case WXK_ESCAPE: EndModal(wxID_CANCEL); return;
            case WXK_LEFT:   MoveSelection_(-1, 0); return;
            case WXK_RIGHT:  MoveSelection_(+1, 0); return;
            case WXK_UP:     MoveSelection_(0, -1); return;
            case WXK_DOWN:   MoveSelection_(0, +1); return;
            case WXK_RETURN:
            case WXK_NUMPAD_ENTER:
                SelectCurrent_();
                return;
            default:
                break;
        }
        e.Skip();
    }
};




bool RunTypeSelectDialog(wxWindow* parent, const Skin& skin, SnakeType initial, SnakeType* outType) {
    if (!outType) return false;
    TypeSelectDialog dlg(parent, skin, initial);
    if (dlg.ShowModal() != wxID_OK) return false;
    *outType = dlg.GetSelection();
    return true;
}

    
