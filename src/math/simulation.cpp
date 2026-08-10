#include "simulation.h"
#include <algorithm>

void Simulation::RunFullSimulation() {
    pullHistory.resize(inputParameters.numOfSimulations);
    unsigned int numCores = std::thread::hardware_concurrency();

    if (numCores == 0) numCores = 8;

    std::vector<std::thread> threads;
    const unsigned int runsPerThread = inputParameters.numOfSimulations / numCores;
    for (unsigned int t = 0; t < numCores; ++t) {
        unsigned int startIndex = t * runsPerThread;
        unsigned int endIndex = (t == numCores - 1) ? inputParameters.numOfSimulations : startIndex + runsPerThread;

        threads.emplace_back([&, startIndex, endIndex]() {
            std::random_device randomNum;
            std::seed_seq seedSequence{ randomNum(), randomNum(), randomNum(), randomNum(), randomNum(), randomNum(), randomNum(), randomNum() };
            std::mt19937 generator(seedSequence);
            for (unsigned int i = startIndex; i < endIndex; ++i) {
                pullHistory[i] = Run(generator, inputParameters.initialGuarantee, inputParameters.numCopiesNeeded, inputParameters.startingNumPulls,
                                                inputParameters.maxPossiblePullsPerSRank, inputParameters.softPityStart, inputParameters.bannerRate,
                                                inputParameters.rampRate, inputParameters.winRate);
            }
            });
    }
    for (auto& thread : threads) thread.join();

    ComputePmfStats();
}
void Simulation::ComputePmfStats() {
    pmf.assign(maxNumberOfSRanks * inputParameters.maxPossiblePullsPerSRank + 1, PmfComparison{});
    PmfCalculator::CalcAnalyticalPmf(pmf, inputParameters.initialGuarantee, inputParameters.numCopiesNeeded, inputParameters.startingNumPulls, 
                                          inputParameters.maxPossiblePullsPerSRank,  inputParameters.softPityStart, inputParameters.bannerRate, 
                                          inputParameters.rampRate, inputParameters.winRate);
    PmfCalculator::CalcSimulatedPmf(pmf, pullHistory, inputParameters.numOfSimulations);
    statistics = StatsCalculator::CalcStatistics(pmf);
}
Simulation Simulation::RunCombined(const SimulationInputParameters& agentParams, const SimulationInputParameters& wengineParams) {
    Simulation agentSim(agentParams);
    agentSim.RunFullSimulation();

    Simulation wengineSim(wengineParams);
    wengineSim.RunFullSimulation();

    Simulation combined;
    combined.inputParameters = agentParams;
    combined.inputParameters.numOfSimulations = std::min(agentParams.numOfSimulations, wengineParams.numOfSimulations);
    combined.inputParameters.startingNumPulls = agentParams.startingNumPulls + wengineParams.startingNumPulls;

    const int agentMax = (int)agentSim.pmf.size() - 1;
    const int wengineMax = (int)wengineSim.pmf.size() - 1;
    combined.pmf.assign(agentMax + wengineMax + 1, PmfComparison{});

    // Convolve both PMFs to get joint.
    for (int i = 1; i <= agentMax; i++) {
        const double pa = agentSim.pmf[i].analytic;
        if (pa <= 0.0) continue;
        for (int j = 1; j <= wengineMax; j++) {
            const double pw = wengineSim.pmf[j].analytic;
            if (pw <= 0.0) continue;
            combined.pmf[i + j].analytic += pa * pw;
        }
    }

    // Simulated: pair up the i-th Agent run with the i-th WEngine run (both are
    // independent Monte Carlo draws of the same size) and sum their totals.
    const size_t n = std::min(agentSim.pullHistory.size(), wengineSim.pullHistory.size());
    for (size_t k = 0; k < n; k++) {
        const int totalPulls = CalcTotalPulls(agentSim.pullHistory[k]) + CalcTotalPulls(wengineSim.pullHistory[k]);
        if (totalPulls >= 0 && totalPulls < (int)combined.pmf.size())
            combined.pmf[totalPulls].simulated += 1.0;
    }
    const double invN = n > 0 ? 1.0 / (double)n : 0.0;
    for (auto& p : combined.pmf) p.simulated *= invN;

    combined.statistics = StatsCalculator::CalcStatistics(combined.pmf);
    combined.pullHistory = std::move(agentSim.pullHistory);
    combined.pullHistoryWEngine = std::move(wengineSim.pullHistory);
    return combined;
}
ArrayOfPullEvents Simulation::Run(std::mt19937& generator, bool initialGuarantee, const int numCompiesNeeded, const int startingNumPulls, 
    const int maxPossiblePullsPerSRank, const int softPityStart, const float bannerRate, const float rampRate, const float winRate) {
    std::uniform_real_distribution<double> dist(0, 1);
    std::uniform_real_distribution<double> winDist(0.0, 1.0);

    int pullsOffset = startingNumPulls;
    int pullsDone = startingNumPulls;
    int totalPullsDone = 0;
    int copiesGained = 0;
    int outcomesRecorded = 0;

    ArrayOfPullEvents pullHistory = {};

    while (copiesGained < numCompiesNeeded && outcomesRecorded < maxNumberOfSRanks) {
        pullsDone++;
        totalPullsDone++;

        double currentRate = bannerRate;
        if (pullsDone >= softPityStart)
            currentRate = bannerRate + rampRate * (pullsDone - (softPityStart - 1));

        if (dist(generator) <= currentRate || pullsDone >= maxPossiblePullsPerSRank) {
            if (winDist(generator) < winRate || initialGuarantee) {
                pullHistory[outcomesRecorded].totalPulls = pullsDone - pullsOffset;
                pullHistory[outcomesRecorded].result = SRankEvent::Win;
                copiesGained++;
                initialGuarantee = false;
            }
            else {
                pullHistory[outcomesRecorded].totalPulls = pullsDone - pullsOffset;
                pullHistory[outcomesRecorded].result = SRankEvent::Lose;
                initialGuarantee = true;
            }
            pullsOffset = 0;
            outcomesRecorded++;
            pullsDone = 0;
        }
    }
    return pullHistory;
}

