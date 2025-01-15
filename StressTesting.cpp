#include "StressTesting.hpp"
#include <random>
#include <cmath>
#include <stdexcept>

StressTesting::StressTestResult StressTesting::runStressTest(
    const Matrix& weights, const Scenario& scenario) {
    
    StressTestResult result;
    
    try {
        // Generate stressed returns
        Matrix stressedReturns = generateStressedReturns(historicalReturns_, scenario);
        
        // Calculate stressed portfolio returns
        Matrix portfolioStressedReturns = stressedReturns * weights;
        
        // Calculate stress test metrics
        result.portfolioReturn = calculateStressedReturn(portfolioStressedReturns);
        result.maxDrawdown = calculateMaxDrawdown(portfolioStressedReturns);
        std::tie(result.var, result.expectedShortfall) = 
            calculateStressedRiskMetrics(portfolioStressedReturns);
        result.factorContributions = 
            calculateFactorContributions(weights, stressedReturns);
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Stress test failed: " + std::string(e.what()));
    }
    
    return result;
}

Matrix StressTesting::generateStressedReturns(
    const Matrix& historicalReturns, const Scenario& scenario) {
    
    Matrix stressedReturns = historicalReturns;
    
    // Apply market shocks
    for (Size i = 0; i < historicalReturns.rows(); ++i) {
        for (Size j = 0; j < historicalReturns.columns(); ++j) {
            stressedReturns[i][j] *= (1.0 + scenario.marketShocks[j]);
        }
    }
    
    // Apply volatility shocks
    Matrix volatility = calculateVolatility(historicalReturns);
    for (Size i = 0; i < volatility.rows(); ++i) {
        volatility[i][0] *= (1.0 + scenario.volatilityShocks[i]);
    }
    
    // Apply correlation shocks
    Matrix correlation = calculateCorrelation(historicalReturns);
    for (Size i = 0; i < correlation.rows(); ++i) {
        for (Size j = 0; j < correlation.columns(); ++j) {
            if (i != j) {
                correlation[i][j] *= (1.0 + scenario.correlationShocks[i * correlation.columns() + j]);
            }
        }
    }
    
    return stressedReturns;
}

std::vector<double> StressTesting::calculateFactorContributions(
    const Matrix& weights, const Matrix& stressedReturns) {
    
    std::vector<double> contributions;
    Matrix factorReturns = decomposeFatorReturns(stressedReturns);
    
    for (Size i = 0; i < factorReturns.columns(); ++i) {
        double contribution = 0.0;
        for (Size j = 0; j < weights.rows(); ++j) {
            contribution += weights[j][0] * factorReturns[j][i];
        }
        contributions.push_back(contribution);
    }
    
    return contributions;
}

double StressTesting::calculateStressedReturn(const Matrix& stressedReturns) {
    double totalReturn = 1.0;
    for (Size i = 0; i < stressedReturns.rows(); ++i) {
        totalReturn *= (1.0 + stressedReturns[i][0]);
    }
    return totalReturn - 1.0;
}

double StressTesting::calculateMaxDrawdown(const Matrix& stressedReturns) {
    double maxDrawdown = 0.0;
    double peak = 1.0;
    double value = 1.0;
    
    for (Size i = 0; i < stressedReturns.rows(); ++i) {
        value *= (1.0 + stressedReturns[i][0]);
        peak = std::max(peak, value);
        maxDrawdown = std::max(maxDrawdown, (peak - value) / peak);
    }
    
    return maxDrawdown;
}

std::tuple<double, double> StressTesting::calculateStressedRiskMetrics(
    const Matrix& stressedReturns) {
    
    std::vector<double> returns;
    returns.reserve(stressedReturns.rows());
    
    for (Size i = 0; i < stressedReturns.rows(); ++i) {
        returns.push_back(stressedReturns[i][0]);
    }
    
    std::sort(returns.begin(), returns.end());
    
    Size varIndex = static_cast<Size>(returns.size() * 0.05); // 95% VaR
    double var = -returns[varIndex];
    
    double es = 0.0;
    for (Size i = 0; i < varIndex; ++i) {
        es += returns[i];
    }
    es = -es / varIndex;
    
    return std::make_tuple(var, es);
}

Matrix StressTesting::calculateVolatility(const Matrix& returns) {
    // Implementation of volatility calculation
    return Matrix();
}

Matrix StressTesting::calculateCorrelation(const Matrix& returns) {
    // Implementation of correlation calculation
    return Matrix();
}

Matrix StressTesting::decomposeFatorReturns(const Matrix& returns) {
    // Implementation of factor decomposition
    return Matrix();
}