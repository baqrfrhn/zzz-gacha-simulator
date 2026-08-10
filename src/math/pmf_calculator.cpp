#include "pmf_calculator.h"

namespace PmfCalculator {
    void CalcAnalyticalPmf(DualPMF& pmf, const bool initialGuarantee, const int numCopiesNeeded, const int initialStart, 
                           const int maxPossiblePullsPerSRank, const int softPityStart, const float bannerRate, const float rampRate, const float winRate) {
        const int startingGuarantee = initialGuarantee ? 1 : 0;
        const int maxPossiblePulls = (maxPossiblePullsPerSRank * 2 * numCopiesNeeded) - initialStart;

        OutcomeGrid3D currentGrid = {};
        OutcomeGrid3D nextGrid = {};

        currentGrid[0][startingGuarantee][initialStart] = 1;


        for (int pullsDone = 1; pullsDone <= maxPossiblePulls; pullsDone++) {
            nextGrid = {};
            for (int copies = 1; copies <= numCopiesNeeded; copies++) {
                for (int guaranteeLevel = 0; guaranteeLevel <= 1; guaranteeLevel++) {
                    for (int pity = 1; pity <= maxPossiblePullsPerSRank; pity++) {
                        double conditioningProb = currentGrid[copies - 1][guaranteeLevel][pity - 1];
                        if (conditioningProb == 0.0) continue;

                        double rateToHit = bannerRate;
                        if (pity >= softPityStart) rateToHit += rampRate * (pity - (softPityStart - 1));
                        if (pity == maxPossiblePullsPerSRank) rateToHit = 1.0;

                        double conditionalProbToMiss = (1.0 - rateToHit);
                        double probabilityToMiss = conditionalProbToMiss * conditioningProb;
                        if (pity < maxPossiblePullsPerSRank)
                            nextGrid[copies - 1][guaranteeLevel][pity] += probabilityToMiss;

                        double probabilityToHit = rateToHit * conditioningProb;
                        if (guaranteeLevel == 1) {
                            if (copies == numCopiesNeeded)
                                pmf[pullsDone].analytic += probabilityToHit;
                            else
                                nextGrid[copies][0][0] += probabilityToHit;
                        }
                        else {
                            float probabilityToWin = probabilityToHit * winRate;
                            float probabilityToLose = probabilityToHit * (1- winRate);

                            nextGrid[copies - 1][1][0] += probabilityToLose;

                            if (copies == numCopiesNeeded)
                                pmf[pullsDone].analytic += probabilityToWin;
                            else
                                nextGrid[copies][0][0] += probabilityToWin;
                        }
                    }
                }
            }
            currentGrid = nextGrid;
        }
    }
    void CalcSimulatedPmf(DualPMF& pmf, const Vector14PullEvents& pullHistory, const int numOfSimulations) {
        for (const auto& part : pullHistory) {
            pmf[CalcTotalPulls(part)].simulated += 1.0;
        }
        const double simCountInv = 1.0 / static_cast<double>(numOfSimulations);
        for (auto& [analytic, simulated] : pmf) {
            simulated *= simCountInv;
        }
    }
}
