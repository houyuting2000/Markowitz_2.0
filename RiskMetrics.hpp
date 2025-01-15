#pragma once
#include <ql/quantlib.hpp>
#include <utility>

using namespace QuantLib;

class RiskMetrics {
public:
    struct PortfolioRisk {
        double var95;          // 95% VaR
        double cvar95;         // Conditional VaR
        double sharpeRatio;    // Sharpe Ratio
        double beta;           // Portfolio Beta
        double treynorRatio;   // Treynor Ratio
        double informationRatio; // Information Ratio
        double maxDrawdown;    // Maximum Drawdown
        double sortino;        // Sortino Ratio
    };

    // Main calculation method
    static PortfolioRisk calculateRiskMetrics(const Matrix& returns, 
                                            const Matrix& weights,
                                            const Matrix& benchmarkReturns);

private:
    static constexpr double RISK_FREE_RATE = 0.02;  // Assumed annual risk-free rate
    static constexpr double CONFIDENCE_LEVEL = 0.95; // For VaR calculation

    // Helper methods
    static std::pair<double, double> calculateVaRCVaR(const Matrix& returns);
    static double calculateSharpeRatio(const Matrix& returns);
    static double calculateBeta(const Matrix& returns, const Matrix& benchmarkReturns);
    static double calculateTreynorRatio(const Matrix& returns, double beta);
    static double calculateInformationRatio(const Matrix& returns, 
                                          const Matrix& benchmarkReturns);
    static double calculateMaxDrawdown(const Matrix& returns);
    static double calculateSortinoRatio(const Matrix& returns);
};