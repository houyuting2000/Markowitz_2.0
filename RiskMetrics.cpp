#include "RiskMetrics.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

RiskMetrics::RiskMetrics(int tradingDaysPerYear) 
    : tradingDaysPerYear_(tradingDaysPerYear)
    , annualizationFactor_(sqrt(tradingDaysPerYear)) {}

RiskMetrics::PortfolioRisk RiskMetrics::calculateRiskMetrics(
    const Matrix& weights,
    const Matrix& returns,
    const Matrix& covariance,
    const Matrix& excessReturns,
    const Matrix& excessCovariance,
    const Matrix& benchmarkReturns,
    double riskFreeRate) {
    
    try {
        PortfolioRisk risk;
        
        // Calculate volatility metrics
        risk.dailyVol = calculateVolatility(weights, covariance, false);
        risk.monthlyVol = risk.dailyVol * sqrt(21);
        risk.annualizedVol = risk.dailyVol * annualizationFactor_;
        risk.trackingError = calculateTrackingError(weights, excessCovariance);
        
        // Calculate portfolio returns
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        double portfolioReturn = std::accumulate(portfolioReturns.begin(), 
                                               portfolioReturns.end(), 0.0) / 
                                               portfolioReturns.size();
        
        double excessReturn = portfolioReturn - riskFreeRate;
        
        // Calculate risk ratios
        risk.beta = calculateBeta(weights, returns, benchmarkReturns);
        risk.alpha = calculateAlpha(weights, returns, benchmarkReturns, riskFreeRate);
        risk.informationRatio = calculateInformationRatio(excessReturn, risk.trackingError);
        risk.sharpeRatio = calculateSharpeRatio(portfolioReturn, risk.dailyVol, riskFreeRate);
        risk.sortino = calculateSortino(weights, returns, riskFreeRate);
        risk.maxDrawdown = calculateMaxDrawdown(weights, returns);
        risk.treynorRatio = calculateTreynorRatio(portfolioReturn, risk.beta, riskFreeRate);
        
        // Calculate VaR and Expected Shortfall
        risk.valueAtRisk = calculateValueAtRisk(weights, returns, params_.confidenceLevel);
        risk.expectedShortfall = calculateExpectedShortfall(weights, returns, params_.confidenceLevel);
        
        return risk;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateRiskMetrics: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateTrackingError(
    const Matrix& weights, 
    const Matrix& excessCovariance) {
    
    try {
        return sqrt((transpose(weights)*excessCovariance*weights)[0][0] * tradingDaysPerYear_);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateTrackingError: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateVolatility(
    const Matrix& weights,
    const Matrix& covariance,
    bool isAnnualized) {
    
    try {
        double vol = sqrt((transpose(weights)*covariance*weights)[0][0]);
        return isAnnualized ? vol * annualizationFactor_ : vol;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateVolatility: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateBeta(
    const Matrix& weights,
    const Matrix& returns,
    const Matrix& benchmarkReturns) {
    
    try {
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        double covar = 0.0, benchmarkVar = 0.0;
        double portfolioMean = 0.0, benchmarkMean = 0.0;
        
        // Calculate means
        for (size_t i = 0; i < portfolioReturns.size(); ++i) {
            portfolioMean += portfolioReturns[i];
            benchmarkMean += benchmarkReturns[i][0];
        }
        portfolioMean /= portfolioReturns.size();
        benchmarkMean /= benchmarkReturns.rows();
        
        // Calculate covariance and variance
        for (size_t i = 0; i < portfolioReturns.size(); ++i) {
            covar += (portfolioReturns[i] - portfolioMean) * 
                    (benchmarkReturns[i][0] - benchmarkMean);
            benchmarkVar += pow(benchmarkReturns[i][0] - benchmarkMean, 2);
        }
        
        covar /= (portfolioReturns.size() - 1);
        benchmarkVar /= (benchmarkReturns.rows() - 1);
        
        return covar / benchmarkVar;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateBeta: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateAlpha(
    const Matrix& weights,
    const Matrix& returns,
    const Matrix& benchmarkReturns,
    double riskFreeRate) {
    
    try {
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        double portfolioReturn = std::accumulate(portfolioReturns.begin(), 
                                               portfolioReturns.end(), 0.0) / 
                                               portfolioReturns.size();
        
        double benchmarkReturn = 0.0;
        for (int i = 0; i < benchmarkReturns.rows(); ++i) {
            benchmarkReturn += benchmarkReturns[i][0];
        }
        benchmarkReturn /= benchmarkReturns.rows();
        
        double beta = calculateBeta(weights, returns, benchmarkReturns);
        
        return portfolioReturn - (riskFreeRate + beta * (benchmarkReturn - riskFreeRate));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateAlpha: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateMaxDrawdown(
    const Matrix& weights,
    const Matrix& returns) {
    
    try {
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        double maxDrawdown = 0.0;
        double peak = 1.0;
        double value = 1.0;
        
        for (double ret : portfolioReturns) {
            value *= (1 + ret);
            peak = std::max(peak, value);
            maxDrawdown = std::min(maxDrawdown, value / peak - 1);
        }
        
        return -maxDrawdown;  // Return positive value
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateMaxDrawdown: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateInformationRatio(
    double excessReturn,
    double trackingError) {
    
    if (trackingError <= 0.0) {
        throw std::runtime_error("Tracking error must be positive");
    }
    return excessReturn / trackingError;
}

double RiskMetrics::calculateSharpeRatio(
    double portfolioReturn,
    double portfolioVol,
    double riskFreeRate) {
    
    if (portfolioVol <= 0.0) {
        throw std::runtime_error("Portfolio volatility must be positive");
    }
    return (portfolioReturn - riskFreeRate) / portfolioVol;
}

double RiskMetrics::calculateSortino(
    const Matrix& weights,
    const Matrix& returns,
    double targetReturn) {
    
    try {
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        double downsideDeviation = calculateDownsideDeviation(weights, returns, targetReturn);
        
        double averageReturn = std::accumulate(portfolioReturns.begin(), 
                                             portfolioReturns.end(), 0.0) / 
                                             portfolioReturns.size();
        
        if (downsideDeviation <= 0.0) {
            throw std::runtime_error("Downside deviation must be positive");
        }
        
        return (averageReturn - targetReturn) / downsideDeviation;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateSortino: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateTreynorRatio(
    double portfolioReturn,
    double beta,
    double riskFreeRate) {
    
    if (std::abs(beta) <= 1e-6) {
        throw std::runtime_error("Beta too close to zero for Treynor ratio");
    }
    return (portfolioReturn - riskFreeRate) / beta;
}

double RiskMetrics::calculateValueAtRisk(
    const Matrix& weights,
    const Matrix& returns,
    double confidenceLevel) {
    
    try {
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        std::sort(portfolioReturns.begin(), portfolioReturns.end());
        
        size_t index = static_cast<size_t>((1 - confidenceLevel) * portfolioReturns.size());
        return -portfolioReturns[index];  // Return positive value
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateValueAtRisk: " + std::string(e.what()));
    }
}

double RiskMetrics::calculateExpectedShortfall(
    const Matrix& weights,
    const Matrix& returns,
    double confidenceLevel) {
    
    try {
        auto portfolioReturns = calculatePortfolioReturns(weights, returns);
        std::sort(portfolioReturns.begin(), portfolioReturns.end());
        
        size_t cutoff = static_cast<size_t>((1 - confidenceLevel) * portfolioReturns.size());
        double es = 0.0;
        
        for (size_t i = 0; i < cutoff; ++i) {
            es += portfolioReturns[i];
        }
        
        return -es / cutoff;  // Return positive value
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateExpectedShortfall: " + std::string(e.what()));
    }
}

std::map<std::string, double> RiskMetrics::calculateFactorExposures(
    const Matrix& weights,
    const Matrix& factorReturns,
    const std::vector<std::string>& factorNames) {
    
    try {
        std::map<std::string, double> exposures;
        Matrix factorBetas = calculateRollingBeta(factorReturns, 
                                                factorReturns, 
                                                factorReturns.rows());
        
        for (size_t i = 0; i < factorNames.size(); ++i) {
            exposures[factorNames[i]] = (transpose(weights) * factorBetas)[0][i];
        }
        
        return exposures;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateFactorExposures: " + std::string(e.what()));
    }
}

Matrix RiskMetrics::calculateRiskContribution(
    const Matrix& weights,
    const Matrix& covariance) {
    
    try {
        Matrix portfolioVol = sqrt((transpose(weights)*covariance*weights));
        return multiply(covariance*weights, weights) / portfolioVol[0][0];
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateRiskContribution: " + std::string(e.what()));
    }
}

Matrix RiskMetrics::calculateComponentVaR(
    const Matrix& weights,
    const Matrix& returns,
    double confidenceLevel) {
    
    try {
        double portfolioVaR = calculateValueAtRisk(weights, returns, confidenceLevel);
        Matrix riskContribution = calculateRiskContribution(weights, returns);
        return multiply(riskContribution, portfolioVaR);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateComponentVaR: " + std::string(e.what()));
    }
}

Matrix RiskMetrics::calculateRollingBeta(
    const Matrix& returns,
    const Matrix& benchmarkReturns,
    int windowSize) {
    
    try {
        int numPeriods = returns.rows() - windowSize + 1;
        Matrix rollingBetas(numPeriods, 1);
        
        for (int i = 0; i < numPeriods; ++i) {
            Matrix windowReturns = returns.block(i, 0, windowSize, returns.columns());
            Matrix windowBenchmark = benchmarkReturns.block(i, 0, windowSize, 1);
            rollingBetas[i][0] = calculateBeta(weights, windowReturns, windowBenchmark);
        }
        
        return rollingBetas;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateRollingBeta: " + std::string(e.what()));
    }
}

Matrix RiskMetrics::calculateRollingVolatility(
    const Matrix& returns,
    int windowSize) {
    
    try {
        int numPeriods = returns.rows() - windowSize + 1;
        Matrix rollingVol(numPeriods, 1);
        
        for (int i = 0; i < numPeriods; ++i) {
            Matrix windowReturns = returns.block(i, 0, windowSize, returns.columns());
            rollingVol[i][0] = calculateVolatility(weights, windowReturns, true);
        }
        
        return rollingVol;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in calculateRollingVolatility: " + std::string(e.what()));
    }
}

// Private helper methods
double RiskMetrics::calculateDownsideDeviation(
    const Matrix& weights,
    const Matrix& returns,
    double targetReturn) {
    
    auto portfolioReturns = calculatePortfolioReturns(weights, returns);
    double sumSquaredDownside = 0.0;
    int count = 0;
    
    for (double ret : portfolioReturns) {
        if (ret < targetReturn) {
            sumSquaredDownside += pow(targetReturn - ret, 2);
            count++;
        }
    }
    
    return count > 0 ? sqrt(sumSquaredDownside / count) : 0.0;
}

std::vector<double> RiskMetrics::calculatePortfolioReturns(
    const Matrix& weights,
    const Matrix& returns) {
    
    std::vector<double> portfolioReturns;
    portfolioReturns.reserve(returns.rows());
    
    for (int i = 0; i < returns.rows(); ++i) {
        double dailyReturn = 0.0;
        for (int j = 0; j < returns.columns(); ++j) {
            dailyReturn += weights[j][0] * returns[i][j];
        }
        portfolioReturns.push_back(dailyReturn);
    }
    
    return portfolioReturns;
}

Matrix RiskMetrics::calculateExponentialCovariance(
    const Matrix& returns,
    double lambda) {
    
    int n = returns.columns();
    Matrix covariance(n, n);
    double sumWeight = 0.0;
    
    for (int i = returns.rows() - 1; i >= 0; --i) {
        double weight = pow(lambda, returns.rows() - 1 - i);
        sumWeight += weight;
        
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                covariance[j][k] += weight * returns[i][j] * returns[i][k];
            }
        }
    }
    
    for (int j = 0; j < n; ++j) {
        for (int k = 0; k < n; ++k) {
            covariance[j][k] /= sumWeight;
        }
    }
    
    return covariance;
}

double RiskMetrics::calculateParametricVaR(
    double mean,
    double stddev,
    double confidenceLevel) {
    
    // Using normal distribution approximation
    double z = InverseCumulativeNormal()(confidenceLevel);
    return -(mean + z * stddev);  // Return positive value
}