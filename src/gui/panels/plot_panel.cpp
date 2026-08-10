#include "ui_panels.h"

void PlotPanel::Draw(AppState& state) {
    if (!state.hasSimulated) {
        ImVec2 sz = ImGui::GetWindowSize();
        const char* line1 = "Configure parameters and run the simulation";
        const char* line2 = "to see your probability distribution.";
        ImVec2 t1 = ImGui::CalcTextSize(line1);
        ImVec2 t2 = ImGui::CalcTextSize(line2);
        float  totalH = t1.y + 8 + t2.y;
        ImGui::SetCursorPos(ImVec2((sz.x - t1.x) * 0.5f, (sz.y - totalH) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
        ImGui::Text("%s", line1);
        ImGui::SetCursorPos(ImVec2((sz.x - t2.x) * 0.5f, ImGui::GetCursorPosY() + 8));
        ImGui::Text("%s", line2);
        ImGui::PopStyleColor();
        return;
    }

    Theme::SectionHeader(state.showCdf ? "CUMULATIVE DISTRIBUTION (CDF)" : "PROBABILITY MASS FUNCTION (PMF)", state.themeConfig.accent);

    // ── Toggle + pull history buttons ─────────────────────────────────────
    {
        const float btnWidth = 200.0f;
        const char* toggleLabel = state.showCdf ? "Switch to PMF     " : "Switch to CDF     ";

        // Use dynamic theme colors instead of hardcoded hex values
        ImGui::PushStyleColor(ImGuiCol_Button, state.themeConfig.buttonBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, state.themeConfig.buttonHovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, state.themeConfig.buttonActive);

        bool toggleClicked = ImGui::Button(toggleLabel, ImVec2(btnWidth, 0));

        ImVec2 btnMin = ImGui::GetItemRectMin();
        ImVec2 btnMax = ImGui::GetItemRectMax();
        float textH = ImGui::GetTextLineHeight();
        float headSz = textH * 0.35f;
        float arrLen = textH * 1.2f;

        ImVec2 cx(btnMax.x - ImGui::GetStyle().FramePadding.x - (arrLen * 0.5f) - 8.0f, btnMin.y + (btnMax.y - btnMin.y) * 0.5f);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
        float y_off = headSz * 0.8f;

        draw->AddLine(ImVec2(cx.x - arrLen * 0.5f, cx.y - y_off), ImVec2(cx.x + arrLen * 0.5f, cx.y - y_off), col, 2.0f);
        draw->AddTriangleFilled(ImVec2(cx.x + arrLen * 0.5f, cx.y - y_off), ImVec2(cx.x + arrLen * 0.5f - headSz, cx.y - y_off - headSz * 0.8f), ImVec2(cx.x + arrLen * 0.5f - headSz, cx.y - y_off + headSz * 0.8f), col);
        draw->AddLine(ImVec2(cx.x + arrLen * 0.5f, cx.y + y_off), ImVec2(cx.x - arrLen * 0.5f, cx.y + y_off), col, 2.0f);
        draw->AddTriangleFilled(ImVec2(cx.x - arrLen * 0.5f, cx.y + y_off), ImVec2(cx.x - arrLen * 0.5f + headSz, cx.y + y_off - headSz * 0.8f), ImVec2(cx.x - arrLen * 0.5f + headSz, cx.y + y_off + headSz * 0.8f), col);

        if (toggleClicked) state.showCdf = !state.showCdf;

        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - btnWidth);
        if (ImGui::Button("View Pull History", ImVec2(btnWidth, 0)))
            ImGui::OpenPopup("PullHistoryPopup");

        ImGui::PopStyleColor(3);
    }

    ImGui::Spacing();

    //const int pmfSz = (int)state.simulation.pmf.size();
    int plotEnd = state.xAxisEnd;
    /*while (plotEnd > 1 && state.simulation.pmf[plotEnd].analytic < 1e-9 &&
        state.simulation.pmf[plotEnd].simulated < 1e-9)
        plotEnd--;
    plotEnd = std::min(plotEnd + 5, pmfSz - 1);*/

    std::vector<double> xs(plotEnd);
    std::vector<double> analyticY(plotEnd), simY(plotEnd);
    std::vector<double> analyticCdf(plotEnd), simCdf(plotEnd);

    double cdfA = 0.0, cdfS = 0.0;
    double maxPmfY = 0.0;

    for (int i = 1; i <= plotEnd; i++) {
        xs[i - 1] = (double)i;
        analyticY[i - 1] = state.simulation.pmf[i].analytic;
        simY[i - 1] = state.simulation.pmf[i].simulated;

        maxPmfY = std::max({ maxPmfY, analyticY[i - 1], simY[i - 1] });

        cdfA += state.simulation.pmf[i].analytic;
        cdfS += state.simulation.pmf[i].simulated;
        analyticCdf[i - 1] = cdfA;
        simCdf[i - 1] = cdfS;
    }
    
    ImVec2 plotSize = ImVec2(-1, -1);

    ImPlot::PushStyleColor(ImPlotCol_PlotBg, state.themeConfig.bg);
    ImPlot::PushStyleColor(ImPlotCol_FrameBg, state.themeConfig.panel);
    ImPlot::PushStyleColor(ImPlotCol_AxisBg, state.themeConfig.panel);

    // Use the dynamic theme border color but lower the alpha for the grid lines
    ImVec4 gridColor = state.themeConfig.border;
    gridColor.w = 0.55f;
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid, gridColor);

    ImPlot::PushStyleColor(ImPlotCol_AxisText, Theme::kTextDim);

    // Use the dynamic panel color with slight transparency for the legend background
    ImVec4 legendBg = state.themeConfig.panel;
    legendBg.w = 0.92f;
    ImPlot::PushStyleColor(ImPlotCol_LegendBg, legendBg);

    ImPlot::PushStyleColor(ImPlotCol_LegendBorder, state.themeConfig.border);

    // Turn off standard crosshairs so we can draw our custom snap-to-point logic
    ImPlotFlags plotFlags = ImPlotFlags_NoTitle | ImPlotFlags_NoMouseText;
    bool plotZoomed = false;
    const double defXMin = 0.0, defXMax = (double)plotEnd;
    const double defPmfYMax = maxPmfY * 1.10;
    const double defCdfYMin = 0.0, defCdfYMax = 1.05;
    char pmfPlotId[32], cdfPlotId[32];
    snprintf(pmfPlotId, sizeof(pmfPlotId), "##pmf_plot_%d", state.plotViewGeneration);
    snprintf(cdfPlotId, sizeof(cdfPlotId), "##cdf_plot_%d", state.plotViewGeneration);

    if (!state.showCdf) {
        if (ImPlot::BeginPlot(pmfPlotId, plotSize, plotFlags)) {
            ImPlot::SetupAxes("Pulls", "Probability");
            ImPlot::SetupAxisLimits(ImAxis_X1, defXMin, defXMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, defPmfYMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -2.0, plotEnd);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -0.005, maxPmfY * 1.25);

            ImPlot::SetupLegend(ImPlotLocation_NorthEast);

            ImPlot::SetNextLineStyle({ state.themeConfig.analytic.x, state.themeConfig.analytic.y, state.themeConfig.analytic.z, 1.0f }, 2.0f);
            ImPlot::PlotStairs("Analytic", xs.data(), analyticY.data(), plotEnd);

            ImPlot::SetNextLineStyle({ state.themeConfig.simulated.x, state.themeConfig.simulated.y, state.themeConfig.simulated.z, 1.0f }, 2.0f);
            ImPlot::PlotStairs("Simulated", xs.data(), simY.data(), plotEnd);

            const int worstCase = state.WorstCasePulls();
            if (state.simulationInputParameters.maxPulls < worstCase) {
                double mpD = (double)state.simulationInputParameters.maxPulls;
                double vx[2] = { mpD, mpD };
                double vy[2] = { 0.0, 1e9 };
                ImPlot::PushStyleColor(ImPlotCol_Line, { 0.95f, 0.35f, 0.35f, 0.80f });
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
                ImPlot::PlotLine("Pull Budget", vx, vy, 2);
                ImPlot::PopStyleVar();
                ImPlot::PopStyleColor();
            }

            // ── Interactive Hover Snapping ────────────────────────────
            if (ImPlot::IsPlotHovered()) {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                int idx = (int)std::round(mouse.x);

                if (idx >= 1 && idx <= plotEnd) {
                    double x_val = xs[idx - 1];
                    // Get the Y value for the specific plot we are in
                    double y_a = state.showCdf ? analyticCdf[idx - 1] : analyticY[idx - 1];
                    double y_s = state.showCdf ? simCdf[idx - 1] : simY[idx - 1];

                    // Draw the crosshair (Vertical AND Horizontal guides)
                    double lx[2] = { x_val, x_val };
                    double ly[2] = { 0.0, ImPlot::GetPlotLimits().Y.Max };

                    double hx[2] = { ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max };
                    double hy[2] = { mouse.y, mouse.y };

                    ImPlot::PushStyleColor(ImPlotCol_Line, { 1.0f, 1.0f, 1.0f, 0.20f });
                    ImPlot::PlotLine("##vguide", lx, ly, 2);
                    ImPlot::PlotLine("##hguide", hx, hy, 2); // The new horizontal line
                    ImPlot::PopStyleColor();

                    // Highlight data points
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.0f, state.themeConfig.analytic, IMPLOT_AUTO, state.themeConfig.analytic);
                    ImPlot::PlotScatter("##pA", &x_val, &y_a, 1);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.0f, state.themeConfig.simulated, IMPLOT_AUTO, state.themeConfig.simulated);
                    ImPlot::PlotScatter("##pS", &x_val, &y_s, 1);

                    ImGui::BeginTooltip();
                    ImGui::Text("Pulls: %d", idx);
                    ImGui::Separator();
                    ImGui::TextColored(state.themeConfig.analytic, "Analytic:  %.4f%%", y_a * 100.0);
                    ImGui::TextColored(state.themeConfig.simulated, "Simulated: %.4f%%", y_s * 100.0);
                    ImGui::EndTooltip();
                }
            }

            ImPlotRect limits = ImPlot::GetPlotLimits();
            // Was comparing against defCdfYMax (~1.0) here, but this is the PMF plot whose
            // Y axis tops out at defPmfYMax (a probability, usually well under 1.0) — that
            // mismatch made isZoomed true almost all the time (even at the default view),
            // so "Reset View" appeared, and clicking it, spuriously all the time.
            bool isZoomed = (limits.X.Min > defXMin + 2.0) || (limits.X.Max < defXMax - 2.0) ||
                (limits.Y.Min > 0.05) || (limits.Y.Max < defPmfYMax * 0.95);

            if (isZoomed) {
                ImVec2 plotPos = ImPlot::GetPlotPos();
                ImGui::SetCursorScreenPos(ImVec2(plotPos.x + 12.0f, plotPos.y + 12.0f));

                ImGui::PushStyleColor(ImGuiCol_Button, state.themeConfig.buttonBg);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, state.themeConfig.buttonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, state.themeConfig.buttonActive);

                if (ImGui::Button("Double click LMB to reset view", ImVec2(225, 0))) {
                    state.plotViewGeneration++;
                }
                ImGui::PopStyleColor(3);
            }

            ImPlot::EndPlot();
        }
    }
    else {
        if (ImPlot::BeginPlot(cdfPlotId, plotSize, plotFlags)) {
            ImPlot::SetupAxes("Pulls", "Cumulative Probability");
            ImPlot::SetupAxisLimits(ImAxis_X1, defXMin, defXMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimits(ImAxis_Y1, defCdfYMin, defCdfYMax, ImPlotCond_Once);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -2.0, plotEnd);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -0.02, 1.10);

            ImPlot::SetupLegend(ImPlotLocation_SouthEast);

            ImPlot::SetNextFillStyle({ state.themeConfig.analytic.x, state.themeConfig.analytic.y, state.themeConfig.analytic.z, 1.0f }, 0.25f);
            ImPlot::SetNextLineStyle({ state.themeConfig.analytic.x, state.themeConfig.analytic.y, state.themeConfig.analytic.z, 1.0f }, 2.0f);
            ImPlot::PlotShaded("Analytic", xs.data(), analyticCdf.data(), plotEnd, 0.0);
            ImPlot::PlotLine("Analytic", xs.data(), analyticCdf.data(), plotEnd);

            ImPlot::SetNextFillStyle({ state.themeConfig.simulated.x, state.themeConfig.simulated.y, state.themeConfig.simulated.z, 1.0f }, 0.20f);
            ImPlot::SetNextLineStyle({ state.themeConfig.simulated.x, state.themeConfig.simulated.y, state.themeConfig.simulated.z, 1.0f }, 2.0f);
            ImPlot::PlotShaded("Simulated", xs.data(), simCdf.data(), plotEnd, 0.0);
            ImPlot::PlotLine("Simulated", xs.data(), simCdf.data(), plotEnd);

            {
                double hx[2] = { xs.front(), xs.back() };
                double hy[2] = { 0.5, 0.5 };
                ImPlot::PushStyleColor(ImPlotCol_Line, { 0.60f, 0.65f, 0.78f, 0.45f });
                ImPlot::PlotLine("50%", hx, hy, 2);
                ImPlot::PopStyleColor();
            }

            const int worstCase = state.WorstCasePulls();
            if (state.simulationInputParameters.maxPulls < worstCase) {
                double mpD = (double)state.simulationInputParameters.maxPulls;
                double vx[2] = { mpD, mpD };
                double vy[2] = { 0.0, 1.05 };
                ImPlot::PushStyleColor(ImPlotCol_Line, { 0.95f, 0.35f, 0.35f, 0.80f });
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
                ImPlot::PlotLine("Pull Budget", vx, vy, 2);
                ImPlot::PopStyleVar();
                ImPlot::PopStyleColor();
            }

            // ── Interactive Hover Snapping ────────────────────────────
            if (ImPlot::IsPlotHovered()) {
                ImPlotPoint mouse = ImPlot::GetPlotMousePos();
                int idx = (int)std::round(mouse.x);

                if (idx >= 1 && idx <= plotEnd) {
                    double x_val = xs[idx - 1];
                    // Get the Y value for the specific plot we are in
                    double y_a = state.showCdf ? analyticCdf[idx - 1] : analyticY[idx - 1];
                    double y_s = state.showCdf ? simCdf[idx - 1] : simY[idx - 1];

                    // Draw the crosshair (Vertical AND Horizontal guides)
                    double lx[2] = { x_val, x_val };
                    double ly[2] = { 0.0, ImPlot::GetPlotLimits().Y.Max };

                    double hx[2] = { ImPlot::GetPlotLimits().X.Min, ImPlot::GetPlotLimits().X.Max };
                    double hy[2] = { mouse.y, mouse.y };

                    ImPlot::PushStyleColor(ImPlotCol_Line, { 1.0f, 1.0f, 1.0f, 0.20f });
                    ImPlot::PlotLine("##vguide", lx, ly, 2);
                    ImPlot::PlotLine("##hguide", hx, hy, 2); // The new horizontal line
                    ImPlot::PopStyleColor();

                    // Highlight data points
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.0f, state.themeConfig.analytic, IMPLOT_AUTO, state.themeConfig.analytic);
                    ImPlot::PlotScatter("##pA", &x_val, &y_a, 1);
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4.0f, state.themeConfig.simulated, IMPLOT_AUTO, state.themeConfig.simulated);
                    ImPlot::PlotScatter("##pS", &x_val, &y_s, 1);

                    ImGui::BeginTooltip();
                    ImGui::Text("Pulls: %d", idx);
                    ImGui::Separator();
                    ImGui::TextColored(state.themeConfig.analytic, "Analytic:  %.4f%%", y_a * 100.0);
                    ImGui::TextColored(state.themeConfig.simulated, "Simulated: %.4f%%", y_s * 100.0);
                    ImGui::EndTooltip();
                }
            }

            ImPlotRect limits = ImPlot::GetPlotLimits();

            bool isZoomed = (limits.X.Min > defXMin + 2.0) || (limits.X.Max < defXMax - 2.0) ||
                (limits.Y.Min > 0.05) || (limits.Y.Max < defCdfYMax * 0.95);

            if (isZoomed) {
                ImVec2 plotPos = ImPlot::GetPlotPos();
                ImGui::SetCursorScreenPos(ImVec2(plotPos.x + 12.0f, plotPos.y + 12.0f));

                ImGui::PushStyleColor(ImGuiCol_Button, state.themeConfig.buttonBg);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, state.themeConfig.buttonHovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, state.themeConfig.buttonActive);

                if (ImGui::Button("Reset View", ImVec2(100, 0))) {
                    state.plotViewGeneration++;
                }
                ImGui::PopStyleColor(3);
            }

            ImPlot::EndPlot();
        }
    }
    ImPlot::PopStyleColor(7);
}
