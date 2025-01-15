#pragma once
#include <ql/quantlib.hpp>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <stdexcept>

using namespace QuantLib;

class RiskMetrics {
public:
    struct PortfolioRisk {
        double dailyVol{0.0};
        double monthlyVol{0.0};
        double annualizedVol{0.0};
        double trackingError{0.0};
        double informationRatio{0.0};
        double sharpeRatio{0.0};
        double sortino{0.0};
        double maxDrawdown{0.0};
        double beta{0.0};
        double alpha{0.0};
        double treynorRatio{0.0};
        double valueAtRisk{0.0};
        double expectedShortfall{0.0};
        std::map<std::string, double> factorExposures;
        
        PortfolioRisk() = default;
    };

    struct RiskParameters {
        double confidenceLevel{0.95};
        int varHorizon{10};
        double targetReturn{0.0};
        bool useExponentialWeighting{false};
        double decayFactor{0.94};
        
        RiskParameters() = default;
    };

    explicit RiskMetrics(int tradingDaysPerYear = 252);
    ~RiskMetrics() = default;

    // Core risk calculations
    PortfolioRisk calculateRiskMetrics(
        const Matrix& weights,
        const Matrix& returns,
        const Matrix& covariance,
        const Matrix& excessReturns,
        const Matrix& excessCovariance,
        const Matrix& benchmarkReturns,
        double riskFreeRate = 0.0);

    // Individual risk measures
    double calculateTrackingError(
        const Matrix& weights, 
        const Matrix& excessCovariance);

    double calculateVolatility(
        const Matrix& weights, 
        const Matrix& covariance,
        bool isAnnualized = false);

    double calculateBeta(
        const Matrix& weights,
        const Matrix& returns,
        const Matrix& benchmarkReturns);

    double calculateAlpha(
        const Matrix& weights,
        const Matrix& returns,
        const Matrix& benchmarkReturns,
        double riskFreeRate);

    double calculateMaxDrawdown(
        const Matrix& weights,
        const Matrix& returns);

    double calculateInformationRatio(
        double excessReturn,
        double trackingError);

    double calculateSharpeRatio(
        double portfolioReturn,
        double portfolioVol,
        double riskFreeRate);

    double calculateSortino(
        const Matrix& weights,
        const Matrix& returns,
        double targetReturn);

    double calculateTreynorRatio(
        double portfolioReturn,
        double beta,
        double riskFreeRate);

    double calculateValueAtRisk(
        const Matrix& weights,
        const Matrix& returns,
        double confidenceLevel = 0.95);

    double calculateExpectedShortfall(
        const Matrix& weights,
        const Matrix& returns,
        double confidenceLevel = 0.95);

    // Factor analysis
    std::map<std::string, double> calculateFactorExposures(
        const Matrix& weights,
        const Matrix& factorReturns,
        const std::vector<std::string>& factorNames);

    // Risk decomposition
    Matrix calculateRiskContribution(
        const Matrix& weights,
        const Matrix& covariance);

    Matrix calculateComponentVaR(
        const Matrix& weights,
        const Matrix& returns,
        double confidenceLevel = 0.95);

    // Rolling analysis
    Matrix calculateRollingBeta(
        const Matrix& returns,
        const Matrix& benchmarkReturns,
        int windowSize);

    Matrix calculateRollingVolatility(
        const Matrix& returns,
        int windowSize);

    // Utility methods
    void setRiskParameters(const RiskParameters& params) { params_ = params; }
    RiskParameters getRiskParameters() const { return params_; }
    void setTradingDaysPerYear(int days) { tradingDaysPerYear_ = days; }
    int getTradingDaysPerYear() const { return tradingDaysPerYear_; }

private:
    int tradingDaysPerYear_;
    double annualizationFactor_;
    RiskParameters params_;

    // Helper methods
    double calculateDownsideDeviation(
        const Matrix& weights,
        const Matrix& returns,
        double targetReturn);

    std::vector<double> calculatePortfolioReturns(
        const Matrix& weights,
        const Matrix& returns);

    Matrix calculateExponentialCovariance(
        const Matrix& returns,
        double lambda);

    double calculateParametricVaR(
        double mean,
        double stddev,
        double confidenceLevel);
};