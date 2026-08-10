#include "ui_panels.h"

void HistoryModal::Draw(const AppState& state) {
    // Dynamically size the window based on whichever leg needs the most copies
    int copiesForWidth = state.IsCombo()
        ? std::max(state.agentComboParams.numCopiesNeeded, state.wengineComboParams.numCopiesNeeded)
        : state.simulationInputParameters.numCopiesNeeded;

    ImGui::SetNextWindowSize(ImVec2(300 + 95 * (2 * copiesForWidth), 520 + 30 * (2 * copiesForWidth)), ImGuiCond_Appearing);

    if (ImGui::BeginPopup("PullHistoryPopup")) {
        ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.accent);
        ImGui::Text("SIMULATION PULL HISTORY");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
        ImGui::TextWrapped("Each row is one Monte Carlo run. Up to %d S-rank events per run (win/loss chain).",
            maxNumberOfSRanks);
        ImGui::PopStyleColor();
        ImGui::Separator();

        const int totalSims = (int)state.simulation.pullHistory.size();

        // ── Navigation & Scroll State ─────────────────────────────────────────
        static int jumpIndex = 1;
        static int lastJumpIndex = 1;
        static bool triggerJump = false;
        static float sharedScrollY = 0.0f;
        static ImGuiID lastActiveTab = 0;

        // Ensure jump index doesn't exceed bounds if a new simulation was run
        jumpIndex = std::clamp(jumpIndex, 1, std::max(1, totalSims));

        ImGui::SetNextItemWidth(140);
        if (ImGui::InputInt("Jump to sim #", &jumpIndex)) {
            jumpIndex = std::clamp(jumpIndex, 1, std::max(1, totalSims));
            if (jumpIndex != lastJumpIndex) {
                triggerJump = true;
                lastJumpIndex = jumpIndex;
            }
        }
        ImGui::SameLine();
        ImGui::Text("of %d", totalSims);
        ImGui::Spacing();

        // ── Reusable Table Builder ────────────────────────────────────────────
        auto drawTable = [&](const Vector14PullEvents& history, const ImVec4& highlightColor, const char* childId) {
            ImGui::BeginChild(childId, ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

            ImGuiID currentTab = ImGui::GetID(childId);

            // Sync scroll when switching tabs
            if (lastActiveTab != currentTab) {
                ImGui::SetScrollY(sharedScrollY);
                lastActiveTab = currentTab;
            }

            // Handle the Jump action
            if (triggerJump) {
                // Approximate row height calculation (Text height + Padding top and bottom)
                float rowHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;
                ImGui::SetScrollY((jumpIndex - 1) * rowHeight);
                triggerJump = false; // Consume jump event
            }

            // Continuously save scroll state to keep the other tab synced
            sharedScrollY = ImGui::GetScrollY();

            if (ImGui::BeginTable("HistoryTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Run #", ImGuiTableColumnFlags_WidthFixed, 70.0f);
                ImGui::TableSetupColumn("Summary", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                ImGui::TableSetupColumn("Event Chain", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                ImGuiListClipper clipper;
                clipper.Begin(totalSims);
                while (clipper.Step()) {
                    for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                        ImGui::TableNextRow();

                        // 1. Run Number Column
                        ImGui::TableSetColumnIndex(0);
                        if (row + 1 == jumpIndex) {
                            ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                        }
                        else {
                            ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
                        }
                        ImGui::Text("#%d", row + 1);
                        ImGui::PopStyleColor();

                        // 2. Summary Column
                        ImGui::TableSetColumnIndex(1);
                        int totalPulls = 0;
                        int eventCount = CountRecordedEvents(history[row]);
                        for (int i = 0; i < eventCount; i++) {
                            totalPulls += history[row][i].totalPulls;
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextPrimary);
                        ImGui::Text("%d pulls, %d events", totalPulls, eventCount);
                        ImGui::PopStyleColor();

                        // 3. Event Chain Column
                        ImGui::TableSetColumnIndex(2);
                        for (int i = 0; i < eventCount; i++) {
                            bool won = history[row][i].result == SRankEvent::Win;

                            if (i > 0) {
                                ImGui::SameLine(0.0f, 0.0f);
                                ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
                                ImGui::Text(" -> ");
                                ImGui::PopStyleColor();
                                ImGui::SameLine(0.0f, 0.0f);
                            }

                            ImGui::PushStyleColor(ImGuiCol_Text, won ? Theme::kGreen : Theme::kRed);
                            ImGui::Text("%s at %d", won ? "Won" : "Lost", history[row][i].totalPulls);
                            ImGui::PopStyleColor();
                        }
                    }
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
            };

        // ── Render Views ──────────────────────────────────────────────────────
        if (state.IsCombo()) {
            if (ImGui::BeginTabBar("HistoryTabs")) {
                if (ImGui::BeginTabItem("Agent")) {
                    drawTable(state.simulation.pullHistory, kAgentTheme.accent, "AgentScroll");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("W-Engine")) {
                    drawTable(state.simulation.pullHistoryWEngine, kWEngineTheme.accent, "WEngineScroll");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        else {
            drawTable(state.simulation.pullHistory, state.themeConfig.accent, "SingleScroll");
        }

        ImGui::EndPopup();
    }
}
int HistoryModal::CountRecordedEvents(const ArrayOfPullEvents& events) const
{
    for (int i = 0; i < maxNumberOfSRanks; i++)
        if (events[i].result == SRankEvent::Indeterminate) return i;
    return maxNumberOfSRanks;
}
