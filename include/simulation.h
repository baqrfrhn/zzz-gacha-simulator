#pragma once
#include "pmf_calculator.h"
#include "stats_calculator.h"
#include <random>
#include <thread>

class Simulation {
public:
    Vector14PullEvents pullHistory{};
    Vector14PullEvents pullHistoryWEngine{}; //for when combo
    StatisticsData statistics{};
    DualPMF pmf{};
    SimulationInputParameters inputParameters{};

    explicit Simulation(const SimulationInputParameters& params = {}) {
        inputParameters = params;
    }
    void RunFullSimulation();

    // Agent -> WEngine combo mode: runs the Agent leg to completion, then the
    // WEngine leg, then combines them into a single "total pulls for both"
    // result — analytic PMF via convolution (the legs are independent), and
    // simulated PMF by summing each leg's total pulls per Monte Carlo run.
    // Per-leg pullHistory is intentionally left empty on the returned
    // Simulation; pull history browsing isn't meaningful for a combined result.
    static Simulation RunCombined(const SimulationInputParameters& agentParams,
                                   const SimulationInputParameters& wengineParams);

private:
    void ComputePmfStats();
    static ArrayOfPullEvents Run(std::mt19937& generator, bool initialGuarantee, const int numCompiesNeeded, const int startingNumPulls, 
        const int maxPossiblePullsPerSRank, const int softPityStart, const float bannerRate, const float rampRate, const float winRate);
};