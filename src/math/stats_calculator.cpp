#include "stats_calculator.h"

namespace StatsCalculator {
    StatisticsData CalcStatistics(const DualPMF& pmf) {
        StatisticsData data;

        data.analyticMean = CalcAnalyticMean(pmf);
        data.simulatedMean = CalcSimulatedMean(pmf);
        data.analyticMedian = CalcAnalyticMedian(pmf);
        data.simulatedMedian = CalcSimulatedMedian(pmf);
        data.analyticVariance = CalcAnalyticVariance(pmf, data.analyticMean);
        data.simulatedVariance = CalcSimulatedVariance(pmf, data.simulatedMean);

        return data;
    }
    double CalcAnalyticMean(const DualPMF& pmf) {
        double mean = 0;
        for (int i = 1; i < pmf.size(); i++) {
            mean += pmf[i].analytic * i;
        }
        return mean;
    }
    double CalcSimulatedMean(const DualPMF& pmf) {
        double mean = 0;
        for (int i = 1; i < pmf.size(); i++) {
            mean += pmf[i].simulated * i;
        }
        return mean;
    }
    int CalcAnalyticMedian(const DualPMF& pmf) {
        double cumulativeProbability = 0.0;

        for (int i = 1; i < pmf.size(); i++) {
            cumulativeProbability += pmf[i].analytic;

            if (cumulativeProbability >= 0.5) {
                return i;
            }
        }

        return 0.0;
    }
    int CalcSimulatedMedian(const DualPMF& pmf) {
        double cumulativeProbability = 0.0;

        for (int i = 1; i < pmf.size(); i++) {
            cumulativeProbability += pmf[i].simulated;

            if (cumulativeProbability >= 0.5) {
                return i;
            }
        }

        return 0.0;
    }
    double CalcAnalyticVariance(const DualPMF& pmf, const double analyticMean) {
        double var = 0;
        for (int i = 1; i < pmf.size(); i++) {
            var += pmf[i].analytic * (i - analyticMean) * (i - analyticMean);
        }
        return var;
    }
    double CalcSimulatedVariance(const DualPMF& pmf, const double simulatedMean) {
        double var = 0;
        for (int i = 1; i < pmf.size(); i++) {
            var += pmf[i].simulated * (i - simulatedMean) * (i - simulatedMean);
        }
        return var;
    }
}