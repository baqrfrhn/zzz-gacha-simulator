#include "theme.h"
#include "implot.h"
#include <algorithm>

namespace Theme {

// ── Fixed constants (semantic, not per-theme) ─────────────────────────────────
const ImVec4 kAccentGold   = { 0.95f, 0.78f, 0.25f, 1.0f };
const ImVec4 kAnalytic     = { 0.35f, 0.72f, 1.00f, 1.0f };
const ImVec4 kSimulated    = { 1.00f, 0.60f, 0.20f, 1.0f };
const ImVec4 kTextPrimary  = { 0.92f, 0.94f, 0.97f, 1.0f };
const ImVec4 kTextDim      = { 0.55f, 0.60f, 0.72f, 1.0f };
const ImVec4 kGreen        = { 0.30f, 0.88f, 0.55f, 1.0f };
const ImVec4 kRed          = { 0.95f, 0.35f, 0.35f, 1.0f };

// ── One-time style setup (shape, spacing — not colours) ───────────────────────
void Apply() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding       = 8.0f;
    s.FrameRounding        = 5.0f;
    s.GrabRounding         = 4.0f;
    s.PopupRounding        = 6.0f;
    s.ScrollbarRounding    = 6.0f;
    s.TabRounding          = 5.0f;
    s.FramePadding         = { 8.0f, 5.0f };
    s.ItemSpacing          = { 10.0f, 7.0f };
    s.WindowPadding        = { 14.0f, 14.0f };
    s.SeparatorTextBorderSize = 2.0f;

    ImPlot::GetStyle().PlotPadding = { 12, 8 };
    ImPlot::GetStyle().LabelPadding = { 6, 4 };
    ImPlot::GetStyle().MinorAlpha = 0.15f;
}

// ── Per-frame colour swap — drives the entire visual theme ────────────────────
void ApplyColors(const ThemeConfig& t) {
    // Tiny helpers for derived shades — avoids adding more ThemeConfig fields.
    auto darken  = [](ImVec4 c, float f) -> ImVec4 {
        return { c.x * f, c.y * f, c.z * f, 1.0f };
    };
    auto lighten = [](ImVec4 c, float a) -> ImVec4 {
        return { std::min(1.0f, c.x + a), std::min(1.0f, c.y + a),
                 std::min(1.0f, c.z + a), 1.0f };
    };

    ImVec4* c = ImGui::GetStyle().Colors;

    // Background & panels
    c[ImGuiCol_WindowBg]      = t.bg;
    c[ImGuiCol_ChildBg]       = t.panel;
    c[ImGuiCol_PopupBg]       = t.panel;
    c[ImGuiCol_Border]        = t.border;

    // Title bars
    c[ImGuiCol_TitleBg]       = darken(t.bg, 0.85f);
    c[ImGuiCol_TitleBgActive] = t.panel;
    c[ImGuiCol_MenuBarBg]     = t.panel;

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]   = t.bg;
    c[ImGuiCol_ScrollbarGrab] = t.border;

    // Input frames (sliders, checkboxes, inputs)
    c[ImGuiCol_FrameBg]        = t.frameBg;
    c[ImGuiCol_FrameBgHovered] = t.frameBgHovered;
    c[ImGuiCol_FrameBgActive]  = t.frameBgActive;

    // Collapsible tree / selectable headers
    c[ImGuiCol_Header]        = t.frameBgActive;
    c[ImGuiCol_HeaderHovered] = t.buttonHovered;
    c[ImGuiCol_HeaderActive]  = t.buttonActive;

    // Neutral buttons (not run, not banner toggle)
    c[ImGuiCol_Button]        = t.buttonBg;
    c[ImGuiCol_ButtonHovered] = t.buttonHovered;
    c[ImGuiCol_ButtonActive]  = t.buttonActive;

    // Accent controls
    c[ImGuiCol_SliderGrab]        = t.accent;
    c[ImGuiCol_SliderGrabActive]  = t.accentBright;
    c[ImGuiCol_CheckMark]         = t.accent;

    // Text — fixed across all themes
    c[ImGuiCol_Text]         = kTextPrimary;
    c[ImGuiCol_TextDisabled] = kTextDim;

    // Separator
    c[ImGuiCol_Separator] = t.border;

    // Tabs
    c[ImGuiCol_Tab]        = t.frameBg;
    c[ImGuiCol_TabHovered] = t.buttonHovered;
    c[ImGuiCol_TabActive]  = t.frameBgActive;

    // Tables
    c[ImGuiCol_TableHeaderBg]     = darken(t.panel, 0.80f);
    c[ImGuiCol_TableRowBg]        = t.panel;
    c[ImGuiCol_TableRowBgAlt]     = lighten(t.panel, 0.02f);
    c[ImGuiCol_TableBorderStrong] = t.border;
    c[ImGuiCol_TableBorderLight]  = t.frameBg;
}

// ── Drawing helpers ───────────────────────────────────────────────────────────
void DrawStar(ImDrawList* draw_list, ImVec2 center, float size, ImU32 col) {
    float inner = size * 0.28f;
    ImVec2 p0 = { center.x,          center.y - size  };
    ImVec2 p1 = { center.x + inner,  center.y - inner };
    ImVec2 p2 = { center.x + size,   center.y         };
    ImVec2 p3 = { center.x + inner,  center.y + inner };
    ImVec2 p4 = { center.x,          center.y + size  };
    ImVec2 p5 = { center.x - inner,  center.y + inner };
    ImVec2 p6 = { center.x - size,   center.y         };
    ImVec2 p7 = { center.x - inner,  center.y - inner };

    draw_list->AddTriangleFilled(p0, p7, p1, col);
    draw_list->AddTriangleFilled(p2, p1, p3, col);
    draw_list->AddTriangleFilled(p4, p3, p5, col);
    draw_list->AddTriangleFilled(p6, p5, p7, col);
    draw_list->AddQuadFilled(p1, p3, p5, p7, col);
}

void SectionHeader(const char* label, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::SeparatorText(label);
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

void StatCard(const char* label, const char* val1, const char* val2,
              const ImVec4& col1, const ImVec4& col2) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushStyleColor(ImGuiCol_Text, kTextDim);
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    ImGui::TableSetColumnIndex(1);
    ImGui::PushStyleColor(ImGuiCol_Text, col1);
    ImGui::Text("%s", val1);
    ImGui::PopStyleColor();

    ImGui::TableSetColumnIndex(2);
    ImGui::PushStyleColor(ImGuiCol_Text, col2);
    ImGui::Text("%s", val2);
    ImGui::PopStyleColor();
}

} // namespace Theme
