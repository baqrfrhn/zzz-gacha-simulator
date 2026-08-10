#pragma once
#include "theme.h"
#include "simulation.h"

inline int LegWorstCasePulls(const SimulationInputParameters& p) {
    return p.maxPossiblePullsPerSRank * 2 * p.numCopiesNeeded;
}
inline int LegMaxAchievablePulls(const SimulationInputParameters& p) {
    return p.initialGuarantee
        ? p.maxPossiblePullsPerSRank * (2 * p.numCopiesNeeded - 1) - p.startingNumPulls
        : p.maxPossiblePullsPerSRank * p.numCopiesNeeded * 2 - p.startingNumPulls;
}

struct AppState {
    SimulationInputParameters simulationInputParameters{};
    Simulation simulation{};
    SimulationType simulationType{};
    BannerConfig bannerConfig{};
    ThemeConfig themeConfig = kAgentTheme;  // initialise to Agent so first frame has valid colours

    SimulationInputParameters agentComboParams{};
    SimulationInputParameters wengineComboParams{};
    int agentPullsTillHardPity = 90;
    int wenginePullsTillHardPity = 80;

    int pullsTillHardPity = 90;
    int xAxisEnd = 180;
    bool hasSimulated = false;
    bool showCdf = false;
    int plotViewGeneration = 0;

    bool IsCombo() const { return simulationType == SimulationType::AgentThenWEngine; }

    int WorstCasePulls() const {
        if (IsCombo())
            return LegWorstCasePulls(agentComboParams) + LegWorstCasePulls(wengineComboParams);
        return LegWorstCasePulls(simulationInputParameters);
    }

    int MaxAchievablePulls() const {
        if (IsCombo())
            return LegMaxAchievablePulls(agentComboParams) + LegMaxAchievablePulls(wengineComboParams);
        return LegMaxAchievablePulls(simulationInputParameters);
    }
};
