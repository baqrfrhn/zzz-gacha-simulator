#pragma once
#include "types.h"

namespace PmfCalculator {
    void CalcAnalyticalPmf(DualPMF& pmf, const bool initialGuarantee, const int numCopiesNeeded, const int initialStart, const int maxPossiblePullsPerSRank, 
        const int softPityStart, const float bannerRate, const float rampRate, const float winRate);
    void CalcSimulatedPmf(DualPMF& pmf, const Vector14PullEvents& pullHistory, const int numOfSimulations);
};