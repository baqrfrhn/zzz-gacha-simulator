#pragma once
#include "ui_panels.h"
#include "raylib.h"
#include "rlImGui.h"

class Gui {
public:
    void StartGui();
private:
    static constexpr int panelWidth  = 1600;
    static constexpr int panelHeight = 900;
    float windowFontScale = 1.3f;
    std::string title = "ZZZ Gacha Simulator";

    AppState state;

    ParameterPanel  parameterPanel;
    BudgetPanel     budgetPanel;
    StatisticsPanel statisticsPanel;
    PlotPanel       plotPanel;
    HistoryModal    historyModal;

    // ── Per-theme fonts ──────────────────────────────────────────────────────
    // Place TTF files at resources/fonts/ relative to your executable.
    // Both fall back to the ImGui default if the files are not found.
    ImFont* m_agentFont   = nullptr;  // e.g. Rajdhani-SemiBold.ttf  — clean, sturdy
    ImFont* m_wengineFont = nullptr;  // e.g. Orbitron-Regular.ttf   — geometric, sci-fi

    void Init();
    void Update();
    void Close();
    void DrawDashboard();
    void DrawTitleBar();
};
