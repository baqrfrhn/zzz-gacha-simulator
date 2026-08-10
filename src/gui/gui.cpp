#include "gui.h"

void Gui::StartGui() { Init(); while (!WindowShouldClose()) { Update(); } Close(); }

void Gui::Init()
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(panelWidth, panelHeight, title.c_str());
    SetWindowMinSize(1280, 720);

    std::string iconPath = std::string(RESOURCES_PATH) + "Zenless_Zone_Zero_logo.png";
    Image icon = LoadImage(iconPath.c_str());;
    SetWindowIcon(icon);
    UnloadImage(icon);

    SetTargetFPS(165);
    rlImGuiSetup(true);
    ImPlot::CreateContext();
    Theme::Apply();
}

void Gui::Close()
{
    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();
}

void Gui::Update()
{
    // Match raylib clear colour to the active theme's background.
    const ImVec4& bg = state.themeConfig.bg;
    BeginDrawing();
    ClearBackground({
        (unsigned char)(bg.x * 255),
        (unsigned char)(bg.y * 255),
        (unsigned char)(bg.z * 255),
        255
    });

    rlImGuiBegin();
    DrawDashboard();
    rlImGuiEnd();

    EndDrawing();
}

void Gui::DrawDashboard()
{
    Theme::ApplyColors(state.themeConfig);

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);

    ImGuiWindowFlags wf = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("Dashboard", nullptr, wf);
    ImGui::SetWindowFontScale(windowFontScale);

    DrawTitleBar();

    // ── Two-column layout ────────────────────────────────────────────────────
    const float leftW = vp->WorkSize.x * 0.36f;
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

    ImGui::BeginChild("LeftPanel", ImVec2(leftW, 0), true);
    if (parameterPanel.Draw(state)) {
        if (state.IsCombo()) {
            // Agent leg first, then WEngine leg, combined into one "total pulls" result.
            state.simulation = Simulation::RunCombined(state.agentComboParams, state.wengineComboParams);
        } else {
            state.simulation.pullHistoryWEngine = {};
            state.simulation = Simulation(state.simulationInputParameters);
            state.simulation.RunFullSimulation();
        }
        state.hasSimulated = true;
        state.plotViewGeneration = 0;
        state.xAxisEnd = state.MaxAchievablePulls();
        if (state.simulationInputParameters.maxPulls > state.xAxisEnd)
            state.simulationInputParameters.maxPulls = state.xAxisEnd;
    }
    budgetPanel.Draw(state);
    statisticsPanel.Draw(state);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("RightPanel", ImVec2(0, 0), true);
    plotPanel.Draw(state);
    historyModal.Draw(state);
    ImGui::EndChild();

    ImGui::PopStyleVar(); // ChildRounding
    ImGui::End();
}

void Gui::DrawTitleBar()
{
    ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.accent);
    ImGui::SetWindowFontScale(windowFontScale * 1.15f);

    ImVec2 startPos = ImGui::GetCursorScreenPos();
    float lineH = ImGui::GetTextLineHeight();

    Theme::DrawStar(ImGui::GetWindowDrawList(),
        ImVec2(startPos.x + lineH * 0.5f, startPos.y + lineH * 0.5f),
        lineH * 0.40f,
        ImGui::GetColorU32(state.themeConfig.accent));

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lineH * 1.2f);
    ImGui::Text("ZZZ Gacha Simulator");

    ImGui::SetWindowFontScale(windowFontScale);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Separator, state.themeConfig.accent);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}
