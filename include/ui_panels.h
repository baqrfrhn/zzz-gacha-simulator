#pragma once
#include "app_state.h"
#include "implot.h"
#include "imgui.h"
#include "theme.h"
#include <string>

// ── Left Panel Components ────────────────────────────────────────────────
const inline float windowFontScale = 1.8f;

class ParameterPanel {
public:
    // Returns true if the "Run Simulation" button was clicked
    bool Draw(AppState& state);
};

class BudgetPanel {
public:
    void Draw(AppState& state);
private:
    // Helper functions
    double CalcCdfAtMaxPulls(const AppState& state) const;
    double CalcSimulatedCdfAtMaxPulls(const AppState& state) const;
};

class StatisticsPanel {
public:
    void Draw(const AppState& state);
};

// ── Right Panel Components ───────────────────────────────────────────────

class PlotPanel {
public:
    void Draw(AppState& state);
};

class HistoryModal {
public:
    void Draw(const AppState& state);
private:
    int CountRecordedEvents(const ArrayOfPullEvents& events) const;
};