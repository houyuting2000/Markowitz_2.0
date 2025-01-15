#pragma once
#include <ql/quantlib.hpp>

class PerformanceAttribution {
public:
    struct Attribution {
        double totalReturn;
        double assetAllocation;
        double securitySelection;
        double interaction;
        std::vector<double> factorContributions;
    };

    Attribution analyzePerformance(const Matrix& weights,
                                 const Matrix& returns,
                                 const Matrix& benchmarkReturns,
                                 const Matrix& factorReturns);

private:
    Matrix decomposeFatorReturns(const Matrix& returns);
    double calculateInformationRatio(const std::vector<double>& excessReturns);
}; 