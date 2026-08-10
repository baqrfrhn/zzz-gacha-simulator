#include "ui_panels.h"

namespace {
    void DrawCopiesAndPitySection(const char* idSuffix, SimulationInputParameters& params,
                                   int& pullsTillHardPity, const ThemeConfig& accentTheme, SimulationType type) {
        ImGui::Spacing();
        ImGui::Text("Mindscape rank");
        for (int i = 1; i <= 7; i++) {
            std::string label = "M" + std::to_string(i - 1) + "##" + idSuffix + std::to_string(i);
            if (i > 1) ImGui::SameLine();
            bool active = (params.numCopiesNeeded == i);
            if (active) ImGui::PushStyleColor(ImGuiCol_Text, accentTheme.accent);
            ImGui::RadioButton(label.c_str(), &params.numCopiesNeeded, i);
            if (active) ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Text("Pulls until hard pity");
        ImGui::SetNextItemWidth(-1);
        if (pullsTillHardPity > params.maxPossiblePullsPerSRank)
            pullsTillHardPity = params.maxPossiblePullsPerSRank;
        std::string pityId = std::string("##pity") + idSuffix;
        ImGui::SliderInt(pityId.c_str(), &pullsTillHardPity, 1, params.maxPossiblePullsPerSRank);
        params.startingNumPulls = params.maxPossiblePullsPerSRank - pullsTillHardPity;

        ImGui::Spacing();
        const char* ratioLabel = (type == SimulationType::WEngine) ? "Lost previous 75/25" : "Lost previous 50/50";
        ImGui::Text("%s", ratioLabel);
        ImGui::SameLine();
        std::string guaranteeId = std::string("##guarantee") + idSuffix;
        ImGui::Checkbox(guaranteeId.c_str(), &params.initialGuarantee);
    }

    void ApplyBannerConfig(SimulationInputParameters& params, const BannerConfig& cfg) {
        params.maxPossiblePullsPerSRank = cfg.hardPity;
        params.softPityStart = cfg.softPityStart;
        params.bannerRate = cfg.baseRate;
        params.rampRate = cfg.rampRate;
        params.winRate = cfg.winRate;
    }
}

bool ParameterPanel::Draw(AppState& state) {
    Theme::SectionHeader("PARAMETERS", state.themeConfig.accent);

    // --- Mode Selector ---
    ImGui::Text("Banner Type");

    auto modeButton = [&](const char* label, SimulationType type, float width) {
        bool active = (state.simulationType == type);
        const ThemeConfig& modeTheme = GetThemeConfig(type);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        ImGui::PushStyleColor(ImGuiCol_Border, modeTheme.accent);

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, modeTheme.accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, modeTheme.accentBright);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, modeTheme.accentBright);
            // Dark text for contrast against the bright accent fill (matches Run button).
            ImGui::PushStyleColor(ImGuiCol_Text, { 0.06f, 0.06f, 0.06f, 1.0f });
        }

        if (ImGui::Button(label, ImVec2(width, 0))) state.simulationType = type;

        if (active) ImGui::PopStyleColor(4);
        ImGui::PopStyleColor(); // Border
        ImGui::PopStyleVar();   // FrameBorderSize
        };

    float avail = ImGui::GetContentRegionAvail().x;
    float thirdW = (avail - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

    modeButton("Agent", SimulationType::Agent, thirdW);
    ImGui::SameLine();
    modeButton("W-Engine", SimulationType::WEngine, thirdW);
    ImGui::SameLine();
    modeButton("Agent + W-Engine", SimulationType::AgentThenWEngine, -1);

    state.themeConfig = GetThemeConfig(state.simulationType);

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
    ImGui::TextWrapped("Ctrl + click a slider to type an exact value");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    if (!state.IsCombo()) {
        // --- Single banner mode (Agent or WEngine) ---
        state.bannerConfig = GetBannerConfig(state.simulationType);
        ApplyBannerConfig(state.simulationInputParameters, state.bannerConfig);

        ImGui::Text("Simulations");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##nsim", &state.simulationInputParameters.numOfSimulations, 100000, 10000000);

        DrawCopiesAndPitySection("single", state.simulationInputParameters, state.pullsTillHardPity,
                                  state.themeConfig, state.simulationType);
    }
    else {
        // --- Agent -> WEngine combo mode: independent inputs for each leg ---
        const BannerConfig agentCfg = GetBannerConfig(SimulationType::Agent);
        const BannerConfig wengineCfg = GetBannerConfig(SimulationType::WEngine);
        ApplyBannerConfig(state.agentComboParams, agentCfg);
        ApplyBannerConfig(state.wengineComboParams, wengineCfg);

        ImGui::Text("Simulations (per leg)");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##nsimcombo", &state.agentComboParams.numOfSimulations, 100000, 10000000);
        state.wengineComboParams.numOfSimulations = state.agentComboParams.numOfSimulations;

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, kAgentTheme.accent);
        ImGui::Text("AGENT:");
        ImGui::PopStyleColor();
        DrawCopiesAndPitySection("agentleg", state.agentComboParams, state.agentPullsTillHardPity,
                                  kAgentTheme, SimulationType::Agent);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, kWEngineTheme.accent);
        ImGui::Text("W-ENGINE:");
        ImGui::PopStyleColor();
        DrawCopiesAndPitySection("wengineleg", state.wengineComboParams, state.wenginePullsTillHardPity,
                                  kWEngineTheme, SimulationType::WEngine);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
        ImGui::TextWrapped("Simulates all Agent copies first, then continues into W-Engine copies. "
                            "Results below reflect total pulls needed for both events.");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Spacing();


    // ── Run button (With geometric drawn Triangle) ─────────────────────
    ImGui::PushStyleColor(ImGuiCol_Button,        state.themeConfig.runBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, state.themeConfig.runHovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  state.themeConfig.runPressed);
    ImGui::PushStyleColor(ImGuiCol_Text, { 0.06f, 0.06f, 0.06f, 1.0f });

    bool runClicked = ImGui::Button("Run Simulation", ImVec2(-1, 0));

    // Draw the play triangle over the button surface
    ImVec2 btnMin = ImGui::GetItemRectMin();
    ImVec2 btnMax = ImGui::GetItemRectMax();
    float btnH = btnMax.y - btnMin.y;
    float triSz = btnH * 0.22f;
    ImVec2 triCenter(btnMin.x + btnH * 0.8f, btnMin.y + btnH * 0.5f);

    ImGui::GetWindowDrawList()->AddTriangleFilled(
        ImVec2(triCenter.x - triSz * 0.6f, triCenter.y - triSz),
        ImVec2(triCenter.x - triSz * 0.6f, triCenter.y + triSz),
        ImVec2(triCenter.x + triSz * 0.8f, triCenter.y),
        ImGui::GetColorU32(ImGuiCol_Text)
    );
    ImGui::PopStyleColor(4);

    return runClicked;

}
