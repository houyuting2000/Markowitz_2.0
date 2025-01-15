#include "RiskReporter.hpp"

void RiskReporter::generateDetailedReport(
    const std::string& filename,
    const RiskMetrics::PortfolioRisk& risk,
    const Matrix& weights,
    const Matrix& returns) {
    
    std::ofstream report(filename);
    if (!report.is_open()) {
        throw std::runtime_error("Unable to open report file: " + filename);
    }

    report << std::fixed << std::setprecision(PRECISION);
    
    // Portfolio Statistics
    report << "Portfolio Statistics\n";
    report << "===================\n\n";
    
    // Portfolio Composition
    report << "Portfolio Composition:\n";
    report << "---------------------\n";
    double totalWeight = 0.0;
    for (Size i = 0; i < weights.rows(); ++i) {
        report << "Asset " << i+1 << ": " << weights[i][0] * 100 << "%\n";
        totalWeight += weights[i][0];
    }
    report << "Total Weight: " << totalWeight * 100 << "%\n\n";
    
    // Risk Metrics
    report << "Risk Metrics:\n";
    report << "-------------\n";
    report << "Value at Risk (95%):    " << risk.var95 * 100 << "%\n";
    report << "Conditional VaR (95%):  " << risk.cvar95 * 100 << "%\n";
    report << "Sharpe Ratio:           " << risk.sharpeRatio << "\n";
    report << "Beta:                   " << risk.beta << "\n";
    report << "Information Ratio:      " << risk.informationRatio << "\n";
    report << "Maximum Drawdown:       " << risk.maxDrawdown * 100 << "%\n";
    report << "Sortino Ratio:         " << risk.sortino << "\n\n";
    
    // Performance Analysis
    report << "Performance Analysis:\n";
    report << "--------------------\n";
    double annualizedReturn = calculateAnnualizedReturn(returns, weights);
    double annualizedVol = calculateAnnualizedVolatility(returns, weights);
    
    report << "Annualized Return:      " << annualizedReturn * 100 << "%\n";
    report << "Annualized Volatility:  " << annualizedVol * 100 << "%\n";
    
    report.close();
}

double RiskReporter::calculateAnnualizedReturn(
    const Matrix& returns, const Matrix& weights) {
    
    Matrix portfolioReturns = returns * weights;
    double meanReturn = 0.0;
    
    for (Size i = 0; i < portfolioReturns.rows(); ++i) {
        meanReturn += portfolioReturns[i][0];
    }
    meanReturn /= portfolioReturns.rows();
    
    return std::pow(1 + meanReturn, TRADING_DAYS_PER_YEAR) - 1;
}

double RiskReporter::calculateAnnualizedVolatility(
    const Matrix& returns, const Matrix& weights) {
    
    Matrix portfolioReturns = returns * weights;
    double meanReturn = 0.0;
    double variance = 0.0;
    
    // Calculate mean
    for (Size i = 0; i < portfolioReturns.rows(); ++i) {
        meanReturn += portfolioReturns[i][0];
    }
    meanReturn /= portfolioReturns.rows();
    
    // Calculate variance
    for (Size i = 0; i < portfolioReturns.rows(); ++i) {
        variance += std::pow(portfolioReturns[i][0] - meanReturn, 2);
    }
    variance /= (portfolioReturns.rows() - 1);
    
    // Annualize volatility
    return std::sqrt(variance * TRADING_DAYS_PER_YEAR);
}