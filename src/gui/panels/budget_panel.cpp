#include "ui_panels.h"

void BudgetPanel::Draw(AppState& state)
{
    if (!state.hasSimulated) return;

    const int worstCasePulls = state.WorstCasePulls();
    int max = state.xAxisEnd;

    ImGui::Spacing();
    ImGui::Spacing();
    Theme::SectionHeader("PULL BUDGET", state.themeConfig.accent);

    ImGui::Spacing();
    ImGui::Text("Pull budget (max pulls)");
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##maxpulls", &state.simulationInputParameters.maxPulls, 1, max);
    
    if (state.simulationInputParameters.maxPulls > max)
        state.simulationInputParameters.maxPulls = max;

    double cdfProb = CalcCdfAtMaxPulls(state);
    double simCdfProb = CalcSimulatedCdfAtMaxPulls(state);
    double pct = cdfProb * 100.0;
    double simPct = simCdfProb * 100.0;
    ImVec4 probCol = (pct >= 75.0) ? Theme::kGreen : (pct >= 40.0 ? Theme::kAccentGold : Theme::kRed);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, state.themeConfig.bg);
    ImGui::BeginChild("BudgetResult", ImVec2(-1, 180), true);
    ImGui::SetWindowFontScale(windowFontScale * 0.88f);

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
    ImGui::TextWrapped("With your pull budget of %d, you have a", state.simulationInputParameters.maxPulls);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, probCol);
    ImGui::SetWindowFontScale(windowFontScale * 1.25f);
    ImGui::Text("  %.2f%%", pct);
    ImGui::SetWindowFontScale(windowFontScale * 0.88f);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
    if (state.IsCombo()) {
        ImGui::TextWrapped("chance to obtain Agent M%d and WEngine R%d (analytic CDF).",
            state.agentComboParams.numCopiesNeeded - 1, state.wengineComboParams.numCopiesNeeded - 1);
    }
    else {
        char rankLabel[16];
        snprintf(rankLabel, sizeof(rankLabel), "M%d", state.simulationInputParameters.numCopiesNeeded - 1);
        ImGui::TextWrapped("chance to obtain all %d copies (%s) (analytic CDF).",
            state.simulationInputParameters.numCopiesNeeded, rankLabel);
    }
    ImGui::Text("Simulated CDF at budget: ");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kSimulated);
    ImGui::Text("%.2f%%", simPct);
    ImGui::PopStyleColor();
    ImGui::Spacing();

    if (state.simulationInputParameters.maxPulls >= worstCasePulls) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::kGreen);
        ImGui::TextWrapped("Budget covers worst-case guarantee (%d pulls).", worstCasePulls);
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, { 0.40f, 0.46f, 0.60f, 1.0f });
        ImGui::TextWrapped("Worst-case guarantee: %d pulls", worstCasePulls);
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleColor(); // kTextDim

    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg
}

double BudgetPanel::CalcCdfAtMaxPulls(const AppState& state) const
{
    double cdf = 0.0;
    const int cap = std::min(state.simulationInputParameters.maxPulls, (int)state.simulation.pmf.size() - 1);
    for (int i = 1; i <= cap; i++)
        cdf += state.simulation.pmf[i].analytic;
    return cdf;
}
double BudgetPanel::CalcSimulatedCdfAtMaxPulls(const AppState& state) const
{
    double cdf = 0.0;
    const int cap = std::min(state.simulationInputParameters.maxPulls, (int)state.simulation.pmf.size() - 1);
    for (int i = 1; i <= cap; i++)
        cdf += state.simulation.pmf[i].simulated;
    return cdf;
}