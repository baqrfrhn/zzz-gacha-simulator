#include "ui_panels.h"

void StatisticsPanel::Draw(const AppState& state) {
    if (!state.hasSimulated) return;

    ImGui::Spacing();
    ImGui::Spacing();
    Theme::SectionHeader("STATISTICS", state.themeConfig.accent);

    auto getPercentile = [&](double p, bool analytic) -> int {
        double cdf = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            cdf += analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            if (cdf >= p) return i;
        }
        return (int)state.simulation.pmf.size() - 1;
        };

    auto getMode = [&](bool analytic) -> int {
        int mode = 0; double maxP = -1.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            if (p > maxP) { maxP = p; mode = i; }
        }
        return mode;
        };

    auto getMinPull = [&](bool analytic) -> int {
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            if (p > 1e-12) return i;
        }
        return 0;
        };

    auto getMaxPull = [&](bool analytic) -> int {
        for (int i = (int)state.simulation.pmf.size() - 1; i >= 1; i--) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            if (p > 1e-12) return i;
        }
        return 0;
        };

    auto getMaxPmf = [&](bool analytic) -> double {
        double maxP = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            maxP = std::max(maxP, p);
        }
        return maxP;
        };

    auto getSkewness = [&](bool analytic) -> double {
        const auto& st = state.simulation.statistics;
        double mean = analytic ? st.analyticMean : st.simulatedMean;
        double var = analytic ? st.analyticVariance : st.simulatedVariance;
        double sd = std::sqrt(var);
        if (sd <= 0.0) return 0.0;
        double skew = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            skew += p * std::pow((i - mean) / sd, 3.0);
        }
        return skew;
        };

    auto getKurtosis = [&](bool analytic) -> double {
        const auto& st = state.simulation.statistics;
        double mean = analytic ? st.analyticMean : st.simulatedMean;
        double var = analytic ? st.analyticVariance : st.simulatedVariance;
        double sd = std::sqrt(var);
        if (sd <= 0.0) return 0.0;
        double kurt = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            kurt += p * std::pow((i - mean) / sd, 4.0);
        }
        return kurt - 3.0;
        };

    auto getEntropy = [&](bool analytic) -> double {
        double h = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            if (p > 1e-12) h -= p * std::log(p);
        }
        return h;
        };

    auto getMad = [&](bool analytic) -> double {
        const auto& st = state.simulation.statistics;
        double mean = analytic ? st.analyticMean : st.simulatedMean;
        double mad = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++) {
            double p = analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
            mad += p * std::fabs(i - mean);
        }
        return mad;
        };

    auto getCdfAt = [&](int pulls, bool analytic) -> double {
        double cdf = 0.0;
        const int cap = std::min(pulls, (int)state.simulation.pmf.size() - 1);
        for (int i = 1; i <= cap; i++)
            cdf += analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
        return cdf;
        };

    auto getProbInRange = [&](double lo, double hi, bool analytic) -> double {
        double prob = 0.0;
        for (int i = 1; i < (int)state.simulation.pmf.size(); i++)
            if (i >= lo && i <= hi)
                prob += analytic ? state.simulation.pmf[i].analytic : state.simulation.pmf[i].simulated;
        return prob;
        };

    ImGuiTableFlags tfl = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("StatsTbl", 3, tfl)) {
        ImGui::TableSetupColumn("Stat", ImGuiTableColumnFlags_WidthStretch, 0.40f);
        ImGui::TableSetupColumn("Analytic", ImGuiTableColumnFlags_WidthStretch, 0.30f);
        ImGui::TableSetupColumn("Simulated", ImGuiTableColumnFlags_WidthStretch, 0.30f);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

        ImGui::TableSetColumnIndex(0);
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);  
        ImGui::Text("Stat");      
        ImGui::PopStyleColor();
        
        ImGui::TableSetColumnIndex(1);
        ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.analytic);
        ImGui::Text("Analytic");
        ImGui::PopStyleColor();

        ImGui::TableSetColumnIndex(2);
        ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.simulated);
        ImGui::Text("Simulated");
        ImGui::PopStyleColor();

        const auto& st = state.simulation.statistics;
        char buf1[48], buf2[48];

        snprintf(buf1, sizeof(buf1), "%.3f", st.analyticMean);
        snprintf(buf2, sizeof(buf2), "%.3f", st.simulatedMean);
        Theme::StatCard("Mean", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%d", static_cast<int>(st.analyticMedian));
        snprintf(buf2, sizeof(buf2), "%d", static_cast<int>(st.simulatedMedian));
        Theme::StatCard("Median", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%d", getMode(true));
        snprintf(buf2, sizeof(buf2), "%d", getMode(false));
        Theme::StatCard("Mode", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.3f", std::sqrt(st.analyticVariance));
        snprintf(buf2, sizeof(buf2), "%.3f", std::sqrt(st.simulatedVariance));
        Theme::StatCard("Std Dev", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.3f", st.analyticVariance);
        snprintf(buf2, sizeof(buf2), "%.3f", st.simulatedVariance);
        Theme::StatCard("Variance", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.4f", st.analyticMean > 0 ? std::sqrt(st.analyticVariance) / st.analyticMean : 0.0);
        snprintf(buf2, sizeof(buf2), "%.4f", st.simulatedMean > 0 ? std::sqrt(st.simulatedVariance) / st.simulatedMean : 0.0);
        Theme::StatCard("CoV (std/mean)", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.3f", getMad(true));
        snprintf(buf2, sizeof(buf2), "%.3f", getMad(false));
        Theme::StatCard("Mean Abs Dev", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%d", getMinPull(true));
        snprintf(buf2, sizeof(buf2), "%d", getMinPull(false));
        Theme::StatCard("Min Pulls", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%d", getMaxPull(true));
        snprintf(buf2, sizeof(buf2), "%d", getMaxPull(false));
        Theme::StatCard("Max Pulls", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%d", getMaxPull(true) - getMinPull(true));
        snprintf(buf2, sizeof(buf2), "%d", getMaxPull(false) - getMinPull(false));
        Theme::StatCard("Range", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        int p_vals[] = { 5, 10, 25, 50, 75, 90, 95, 99 };
        const char* p_labels[] = {
            "P5", "P10", "P25 (Q1)", "P50 (Median)", "P75 (Q3)", "P90", "P95", "P99"
        };
        for (int i = 0; i < 8; i++) {
            snprintf(buf1, sizeof(buf1), "%d", getPercentile(p_vals[i] / 100.0, true));
            snprintf(buf2, sizeof(buf2), "%d", getPercentile(p_vals[i] / 100.0, false));
        Theme::StatCard(p_labels[i], buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);
        }

        int iqrA = getPercentile(0.75, true) - getPercentile(0.25, true);
        int iqrS = getPercentile(0.75, false) - getPercentile(0.25, false);
        snprintf(buf1, sizeof(buf1), "%d", iqrA);
        snprintf(buf2, sizeof(buf2), "%d", iqrS);
        Theme::StatCard("IQR (P75-P25)", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.4f", getSkewness(true));
        snprintf(buf2, sizeof(buf2), "%.4f", getSkewness(false));
        Theme::StatCard("Skewness", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.4f", getKurtosis(true));
        snprintf(buf2, sizeof(buf2), "%.4f", getKurtosis(false));
        Theme::StatCard("Excess Kurtosis", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.4f", getEntropy(true));
        snprintf(buf2, sizeof(buf2), "%.4f", getEntropy(false));
        Theme::StatCard("Entropy (nats)", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.4f%%", getMaxPmf(true) * 100.0);
        snprintf(buf2, sizeof(buf2), "%.4f%%", getMaxPmf(false) * 100.0);
        Theme::StatCard("Peak PMF", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        snprintf(buf1, sizeof(buf1), "%.3f", st.analyticMean - st.analyticMedian);
        snprintf(buf2, sizeof(buf2), "%.3f", st.simulatedMean - st.simulatedMedian);
        Theme::StatCard("Mean - Median", buf1, buf2, state.themeConfig.analytic, state.themeConfig.simulated);

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.09f, 0.11f, 0.18f, 1.0f });
    ImGui::BeginChild("ExtraStats", ImVec2(-1, 0), true);
    ImGui::SetWindowFontScale(windowFontScale * 0.88f);

    Theme::SectionHeader("POLYCHROME COST (ESTIMATES)", state.themeConfig.accent);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);

    int medianPulls = getPercentile(0.50, true);
    int p90Pulls = getPercentile(0.90, true);
    int p95Pulls = getPercentile(0.95, true);
    int p99Pulls = getPercentile(0.99, true);
    const int worstCasePulls = state.WorstCasePulls();

    ImGui::Text("Mean:"); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.accent);
    ImGui::Text("~%d", (int)std::round(state.simulation.statistics.analyticMean * 160));
    ImGui::PopStyleColor();

    ImGui::Text("Median (P50):  "); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.accent);
    ImGui::Text(" %d", medianPulls * 160);
    ImGui::PopStyleColor();

    ImGui::Text("Unlucky (P90): "); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kRed);
    ImGui::Text(" %d", p90Pulls * 160);
    ImGui::PopStyleColor();

    ImGui::Text("Very unlucky (P95):"); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kRed);
    ImGui::Text(" %d", p95Pulls * 160);
    ImGui::PopStyleColor();

    ImGui::Text("Extreme (P99):   "); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kRed);
    ImGui::Text(" %d", p99Pulls * 160);
    ImGui::PopStyleColor();

    ImGui::Text("Budget cap:      "); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, state.themeConfig.accent);
    ImGui::Text(" %d", state.simulationInputParameters.maxPulls * 160);
    ImGui::PopStyleColor();

    ImGui::Text("Absolute Worst:"); ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kRed);
    ImGui::Text(" %d", worstCasePulls * 160);
    ImGui::PopStyleColor();

    ImGui::PopStyleColor(); // kTextDim

    ImGui::Spacing();
    Theme::SectionHeader("SIMULATION METADATA", state.themeConfig.accent);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::kTextDim);
    if (state.IsCombo()) {
        ImGui::Text("Runs/leg: %d  |  Agent copies: %d  |  WEngine copies: %d",
            state.agentComboParams.numOfSimulations,
            state.agentComboParams.numCopiesNeeded,
            state.wengineComboParams.numCopiesNeeded);
        ImGui::Text("Starting pity offset: %d pulls (both legs)  |  Agent guarantee: %s  |  WEngine guarantee: %s",
            state.simulation.inputParameters.startingNumPulls,
            state.agentComboParams.initialGuarantee ? "yes" : "no",
            state.wengineComboParams.initialGuarantee ? "yes" : "no");
    }
    else {
        ImGui::Text("Runs: %d  |  Copies needed: %d  |  Max S-rank events/run: %d",
            state.simulationInputParameters.numOfSimulations,
            state.simulationInputParameters.numCopiesNeeded,
            state.simulationInputParameters.numCopiesNeeded * 2);
        ImGui::Text("Starting pity offset: %d pulls  |  Initial 50/50 guarantee: %s",
            state.simulationInputParameters.startingNumPulls,
            state.simulationInputParameters.initialGuarantee ? "yes" : "no");
    }
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor(); // ChildBg
}
