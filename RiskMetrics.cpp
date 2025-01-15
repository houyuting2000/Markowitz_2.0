#include "RiskMetrics.hpp"
#include <numeric>
#include <algorithm>
#include <cmath>
#include <vector>

RiskMetrics::PortfolioRisk RiskMetrics::calculateRiskMetrics(
    const Matrix& returns, const Matrix& weights, const Matrix& benchmarkReturns) {
    
    PortfolioRisk risk;
    
    // Calculate portfolio returns
    Matrix portfolioReturns = returns * weights;
    
    // Calculate VaR and CVaR
    std::tie(risk.var95, risk.cvar95) = calculateVaRCVaR(portfolioReturns);
    
    // Calculate Sharpe Ratio
    risk.sharpeRatio = calculateSharpeRatio(portfolioReturns);
    
    // Calculate Beta
    risk.beta = calculateBeta(portfolioReturns, benchmarkReturns);
    
    // Calculate Treynor Ratio
    risk.treynorRatio = calculateTreynorRatio(portfolioReturns, risk.beta);
    
    // Calculate Information Ratio
    risk.informationRatio = calculateInformationRatio(portfolioReturns, benchmarkReturns);
    
    // Calculate Maximum Drawdown
    risk.maxDrawdown = calculateMaxDrawdown(portfolioReturns);
    
    // Calculate Sortino Ratio
    risk.sortino = calculateSortinoRatio(portfolioReturns);
    
    return risk;
}

std::pair<double, double> RiskMetrics::calculateVaRCVaR(const Matrix& returns) {
    std::vector<double> sortedReturns;
    sortedReturns.reserve(returns.rows());
    
    for (Size i = 0; i < returns.rows(); ++i) {
        sortedReturns.push_back(returns[i][0]);
    }
    std::sort(sortedReturns.begin(), sortedReturns.end());
    
    Size varIndex = static_cast<Size>(sortedReturns.size() * (1 - CONFIDENCE_LEVEL));
    double var = -sortedReturns[varIndex];
    
    double cvar = 0.0;
    for (Size i = 0; i < varIndex; ++i) {
        cvar += sortedReturns[i];
    }
    cvar = -cvar / varIndex;
    
    return {var, cvar};
}

double RiskMetrics::calculateSharpeRatio(const Matrix& returns) {
    double meanReturn = 0.0;
    double variance = 0.0;
    
    // Calculate mean return
    for (Size i = 0; i < returns.rows(); ++i) {
        meanReturn += returns[i][0];
    }
    meanReturn /= returns.rows();
    
    // Calculate variance
    for (Size i = 0; i < returns.rows(); ++i) {
        variance += std::pow(returns[i][0] - meanReturn, 2);
    }
    variance /= (returns.rows() - 1);
    
    // Annualize (assuming daily returns)
    double annualizedReturn = meanReturn * 252;
    double annualizedVol = std::sqrt(variance * 252);
    
    return (annualizedReturn - RISK_FREE_RATE) / annualizedVol;
}

double RiskMetrics::calculateBeta(const Matrix& returns, const Matrix& benchmarkReturns) {
    double covariance = 0.0;
    double benchmarkVariance = 0.0;
    
    double meanReturn = 0.0;
    double meanBenchmark = 0.0;
    
    // Calculate means
    for (Size i = 0; i < returns.rows(); ++i) {
        meanReturn += returns[i][0];
        meanBenchmark += benchmarkReturns[i][0];
    }
    meanReturn /= returns.rows();
    meanBenchmark /= returns.rows();
    
    // Calculate covariance and benchmark variance
    for (Size i = 0; i < returns.rows(); ++i) {
        covariance += (returns[i][0] - meanReturn) * 
                     (benchmarkReturns[i][0] - meanBenchmark);
        benchmarkVariance += std::pow(benchmarkReturns[i][0] - meanBenchmark, 2);
    }
    
    covariance /= (returns.rows() - 1);
    benchmarkVariance /= (returns.rows() - 1);
    
    return covariance / benchmarkVariance;
}

double RiskMetrics::calculateTreynorRatio(const Matrix& returns, double beta) {
    double meanReturn = 0.0;
    for (Size i = 0; i < returns.rows(); ++i) {
        meanReturn += returns[i][0];
    }
    meanReturn /= returns.rows();
    
    double annualizedReturn = meanReturn * 252;
    return (annualizedReturn - RISK_FREE_RATE) / beta;
}

double RiskMetrics::calculateInformationRatio(
    const Matrix& returns, const Matrix& benchmarkReturns) {
    
    Matrix excessReturns(returns.rows(), 1);
    for (Size i = 0; i < returns.rows(); ++i) {
        excessReturns[i][0] = returns[i][0] - benchmarkReturns[i][0];
    }
    
    double meanExcess = 0.0;
    double variance = 0.0;
    
    for (Size i = 0; i < excessReturns.rows(); ++i) {
        meanExcess += excessReturns[i][0];
    }
    meanExcess /= excessReturns.rows();
    
    for (Size i = 0; i < excessReturns.rows(); ++i) {
        variance += std::pow(excessReturns[i][0] - meanExcess, 2);
    }
    variance /= (excessReturns.rows() - 1);
    
    return meanExcess / std::sqrt(variance);
}

double RiskMetrics::calculateMaxDrawdown(const Matrix& returns) {
    double maxDrawdown = 0.0;
    double peak = 1.0;
    double value = 1.0;
    
    for (Size i = 0; i < returns.rows(); ++i) {
        value *= (1 + returns[i][0]);
        peak = std::max(peak, value);
        maxDrawdown = std::max(maxDrawdown, (peak - value) / peak);
    }
    
    return maxDrawdown;
}

double RiskMetrics::calculateSortinoRatio(const Matrix& returns) {
    double meanReturn = 0.0;
    double downsideVariance = 0.0;
    
    for (Size i = 0; i < returns.rows(); ++i) {
        meanReturn += returns[i][0];
    }
    meanReturn /= returns.rows();
    
    for (Size i = 0; i < returns.rows(); ++i) {
        if (returns[i][0] < 0) {
            downsideVariance += std::pow(returns[i][0], 2);
        }
    }
    downsideVariance /= (returns.rows() - 1);
    
    double annualizedReturn = meanReturn * 252;
    double annualizedDownsideVol = std::sqrt(downsideVariance * 252);
    
    return (annualizedReturn - RISK_FREE_RATE) / annualizedDownsideVol;
}