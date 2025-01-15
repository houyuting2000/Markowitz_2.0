#pragma once
#include <ql/quantlib.hpp>
#include <vector>
#include <string>
#include <tuple>

using namespace QuantLib;

class StressTesting {
public:
    struct Scenario {
        std::string name;
        std::vector<double> marketShocks;
        std::vector<double> volatilityShocks;
        std::vector<double> correlationShocks;
    };

    struct StressTestResult {
        double portfolioReturn;
        double maxDrawdown;
        double var;
        double expectedShortfall;
        std::vector<double> factorContributions;
    };

    // Constructor
    StressTesting(const Matrix& historicalReturns) 
        : historicalReturns_(historicalReturns) {}

    // Main stress testing method
    StressTestResult runStressTest(const Matrix& weights,
                                 const Scenario& scenario);

private:
    // Member variables
    Matrix historicalReturns_;

    // Helper methods
    Matrix generateStressedReturns(const Matrix& historicalReturns,
                                 const Scenario& scenario);
    
    double calculateStressedReturn(const Matrix& stressedReturns);
    double calculateMaxDrawdown(const Matrix& stressedReturns);
    std::tuple<double, double> calculateStressedRiskMetrics(const Matrix& stressedReturns);
    std::vector<double> calculateFactorContributions(const Matrix& weights,
                                                   const Matrix& stressedReturns);
    
    // Additional helper methods
    Matrix calculateVolatility(const Matrix& returns);
    Matrix calculateCorrelation(const Matrix& returns);
    Matrix decomposeFatorReturns(const Matrix& returns);
};