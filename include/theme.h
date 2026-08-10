#pragma once
#include "types.h"
#include "imgui.h"

// ─────────────────────────────────────────────────────────────────────────────
//  ThemeConfig  —  everything that differs between Agent and W-Engine themes.
//  Passed into ApplyColors() each frame; zero cost, just array writes.
// ─────────────────────────────────────────────────────────────────────────────
struct ThemeConfig {
    // Accent / interactive highlights
    ImVec4 accent;              // slider grab, section headers, active markers
    ImVec4 accentBright;        // slider grab (dragging)

    // Banner toggle buttons (Agent / WEngine selector)
    ImVec4 btnActiveBg;         // bg when this banner button is selected
    ImVec4 btnActiveHovered;
    ImVec4 btnActivePressed;

    // "Run Simulation" button
    ImVec4 runBg;
    ImVec4 runHovered;
    ImVec4 runPressed;

    // World colours — background, panels, frames, neutral buttons
    ImVec4 bg;                  // main viewport / window background
    ImVec4 panel;               // child panels, popups
    ImVec4 border;              // borders, separators, legend borders
    ImVec4 frameBg;             // input fields, slider track, checkbox bg
    ImVec4 frameBgHovered;
    ImVec4 frameBgActive;
    ImVec4 buttonBg;            // neutral (non-run, non-banner) buttons
    ImVec4 buttonHovered;
    ImVec4 buttonActive;

    ImVec4 analytic;            // Color for analytic plot lines/data
    ImVec4 simulated;           // Color for simulated plot lines/data
};

// ── Agent: deep navy blue world + gold ───────────────────────────────────────
inline const ThemeConfig kAgentTheme {
    { 0.95f, 0.78f, 0.25f, 1.0f },   // accent         (gold)
    { 1.00f, 0.88f, 0.40f, 1.0f },   // accentBright
    { 0.32f, 0.23f, 0.02f, 1.0f },   // btnActiveBg
    { 0.42f, 0.31f, 0.05f, 1.0f },   // btnActiveHovered
    { 0.50f, 0.37f, 0.07f, 1.0f },   // btnActivePressed
    { 0.55f, 0.40f, 0.05f, 1.0f },   // runBg
    { 0.75f, 0.56f, 0.10f, 1.0f },   // runHovered
    { 0.90f, 0.68f, 0.15f, 1.0f },   // runPressed
    { 0.07f, 0.08f, 0.12f, 1.0f },   // bg             (deep navy)
    { 0.10f, 0.12f, 0.18f, 1.0f },   // panel
    { 0.20f, 0.25f, 0.38f, 1.0f },   // border
    { 0.13f, 0.16f, 0.24f, 1.0f },   // frameBg
    { 0.18f, 0.22f, 0.33f, 1.0f },   // frameBgHovered
    { 0.22f, 0.27f, 0.40f, 1.0f },   // frameBgActive
    { 0.18f, 0.22f, 0.35f, 1.0f },   // buttonBg
    { 0.26f, 0.34f, 0.55f, 1.0f },   // buttonHovered
    { 0.32f, 0.42f, 0.65f, 1.0f },   // buttonActive
    { 0.35f, 0.72f, 1.00f, 1.0f },   // analytic (blue)
    { 1.00f, 0.60f, 0.20f, 1.0f }    // simulated (orange)
};

// ── W-Engine: deep purple world + electric lime ───────────────────────────────
inline const ThemeConfig kWEngineTheme {
    { 0.68f, 1.00f, 0.12f, 1.0f },   // accent         (electric lime)
    { 0.80f, 1.00f, 0.30f, 1.0f },   // accentBright
    { 0.07f, 0.22f, 0.02f, 1.0f },   // btnActiveBg    (dark lime tint)
    { 0.10f, 0.32f, 0.04f, 1.0f },   // btnActiveHovered
    { 0.13f, 0.40f, 0.06f, 1.0f },   // btnActivePressed
    { 0.28f, 0.10f, 0.54f, 1.0f },   // runBg          (vivid purple)
    { 0.38f, 0.16f, 0.70f, 1.0f },   // runHovered
    { 0.46f, 0.20f, 0.82f, 1.0f },   // runPressed
    { 0.07f, 0.05f, 0.13f, 1.0f },   // bg             (deep purple-dark)
    { 0.12f, 0.08f, 0.22f, 1.0f },   // panel
    { 0.40f, 0.20f, 0.65f, 1.0f },   // border         (vivid purple)
    { 0.15f, 0.10f, 0.28f, 1.0f },   // frameBg
    { 0.22f, 0.14f, 0.40f, 1.0f },   // frameBgHovered
    { 0.28f, 0.18f, 0.50f, 1.0f },   // frameBgActive
    { 0.17f, 0.10f, 0.32f, 1.0f },   // buttonBg
    { 0.28f, 0.16f, 0.52f, 1.0f },   // buttonHovered
    { 0.35f, 0.20f, 0.62f, 1.0f },   // buttonActive
    { 0.15f, 0.85f, 0.85f, 1.0f },   // analytic (neon cyan)
    { 0.95f, 0.25f, 0.65f, 1.0f }    // simulated (hot pink)
};

// ── Agent → WEngine (combo): graphite fusion + champagne gold ────────────────
// Its own theme, distinct from both single-banner themes, since it's a mode
// in its own right rather than "whichever banner happens to be selected".
inline const ThemeConfig kComboTheme {
    { 0.90f, 0.80f, 0.55f, 1.0f },   // accent         (champagne gold)
    { 1.00f, 0.90f, 0.65f, 1.0f },   // accentBright
    { 0.28f, 0.24f, 0.14f, 1.0f },   // btnActiveBg
    { 0.38f, 0.32f, 0.18f, 1.0f },   // btnActiveHovered
    { 0.46f, 0.39f, 0.22f, 1.0f },   // btnActivePressed
    { 0.32f, 0.20f, 0.42f, 1.0f },   // runBg          (fused navy-purple)
    { 0.44f, 0.28f, 0.56f, 1.0f },   // runHovered
    { 0.55f, 0.36f, 0.68f, 1.0f },   // runPressed
    { 0.06f, 0.06f, 0.08f, 1.0f },   // bg             (graphite / obsidian)
    { 0.10f, 0.10f, 0.14f, 1.0f },   // panel
    { 0.32f, 0.28f, 0.34f, 1.0f },   // border
    { 0.14f, 0.13f, 0.18f, 1.0f },   // frameBg
    { 0.20f, 0.18f, 0.26f, 1.0f },   // frameBgHovered
    { 0.25f, 0.22f, 0.32f, 1.0f },   // frameBgActive
    { 0.18f, 0.16f, 0.22f, 1.0f },   // buttonBg
    { 0.28f, 0.24f, 0.36f, 1.0f },   // buttonHovered
    { 0.34f, 0.30f, 0.44f, 1.0f },   // buttonActive
    { 1.00f, 0.65f, 0.25f, 1.0f },   // analytic (amber)
    { 0.65f, 0.45f, 1.00f, 1.0f }    // simulated (violet)
};

inline const ThemeConfig& GetThemeConfig(SimulationType type) {
    switch (type) {
        case SimulationType::Agent:   return kAgentTheme;
        case SimulationType::WEngine: return kWEngineTheme;
        default:                      return kComboTheme;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Theme namespace  —  fixed constants (same across all themes) + API
// ─────────────────────────────────────────────────────────────────────────────
namespace Theme {
    // Fixed semantic colours — never theme-dependent
    extern const ImVec4 kAccentGold;    // "warning/medium" in traffic-light UIs
    extern const ImVec4 kAnalytic;      // analytic line / column (blue)
    extern const ImVec4 kSimulated;     // simulated line / column (orange)
    extern const ImVec4 kTextPrimary;
    extern const ImVec4 kTextDim;
    extern const ImVec4 kGreen;
    extern const ImVec4 kRed;

    void Apply();                           // one-time style setup (roundings, padding)
    void ApplyColors(const ThemeConfig& t); // swap ALL ImGui colours — call each frame

    void DrawStar(ImDrawList* draw_list, ImVec2 center, float size, ImU32 col);
    void SectionHeader(const char* label, const ImVec4& color = kAccentGold);
    void StatCard(const char* label, const char* val1, const char* val2,
                  const ImVec4& col1, const ImVec4& col2);
}
