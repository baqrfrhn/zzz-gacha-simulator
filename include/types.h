#pragma once
#include <vector>
#include <array>

enum class SimulationType {
    Agent, WEngine, AgentThenWEngine
};
enum class SRankEvent {
    Indeterminate, Win, Lose
};
struct PmfComparison {
    double analytic = 0.0;
    double simulated = 0.0;
};
struct PullEvent {
    int totalPulls = 0;
    SRankEvent result = SRankEvent::Indeterminate;
};
struct SimulationInputParameters {
    int numOfSimulations = 1000000;
    bool initialGuarantee = false;
    int numCopiesNeeded = 1;
    int startingNumPulls = 0;
    int maxPulls = 180;
    int maxPossiblePullsPerSRank = 90;
    int softPityStart = 74;
    float rampRate = 0.06f;
    float bannerRate = 0.006f;
    float winRate = 0.5f;
};
struct StatisticsData {
    double analyticMean = 0.0;
    double simulatedMean = 0.0;
    double analyticMedian = 0.0;
    double simulatedMedian = 0.0;
    double analyticVariance = 0.0;
    double simulatedVariance = 0.0;
};

struct BannerConfig {
    int   hardPity;
    int   softPityStart;
    float baseRate;
    float rampRate;
    float winRate;
};
inline constexpr BannerConfig kAgentConfig{ 90, 74, 0.006f, 0.06f, 0.50f };
inline constexpr BannerConfig kWEngineConfig{ 80, 65, 0.010f, 0.06f, 0.75f };
// Note: AgentThenWEngine (combo mode) drives two independent legs, each with its
// own BannerConfig (kAgentConfig / kWEngineConfig) — this accessor is only
// meaningful for the two single-banner modes.
inline const BannerConfig& GetBannerConfig(SimulationType type) {
    return (type == SimulationType::Agent) ? kAgentConfig : kWEngineConfig;
}

inline constexpr int maxNumberOfSRanks = 14;
inline constexpr int maxCopiesPossible = 7;
using ArrayOfPullEvents = std::array<PullEvent, maxNumberOfSRanks>;
using Vector14PullEvents = std::vector<ArrayOfPullEvents>;
using OutcomeGrid3D = std::array<std::array<std::array<double, 90>, 2>, maxCopiesPossible>;
using DualPMF = std::vector<PmfComparison>;

inline int CalcTotalPulls(const ArrayOfPullEvents& simulation) {
    int count = 0;
    for (int i = 0; i < 14; i++) {
        count += simulation[i].totalPulls;
    }
    return count;
}

