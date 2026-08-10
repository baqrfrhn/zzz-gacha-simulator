#pragma once
#include "types.h"
#include <cmath>


namespace StatsCalculator{
    StatisticsData CalcStatistics(const DualPMF& pmf);

    double CalcAnalyticMean(const DualPMF& pmf);
    double CalcSimulatedMean(const DualPMF& pmf);
    int CalcAnalyticMedian(const DualPMF& pmf);
    int CalcSimulatedMedian(const DualPMF& pmf);
    double CalcAnalyticVariance(const DualPMF& pmf, const double analyticMean);
    double CalcSimulatedVariance(const DualPMF& pmf, const double simulatedMean);
};