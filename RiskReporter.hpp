#pragma once
#include "RiskMetrics.hpp"
#include <fstream>
#include <iomanip>
#include <stdexcept>

class RiskReporter {
public:
    // Main report generation methods
    static void generateDetailedReport(const std::string& filename,
                                     const RiskMetrics::PortfolioRisk& risk,
                                     const Matrix& weights,
                                     const Matrix& returns);

private:
    // Helper methods for calculations
    static double calculateAnnualizedReturn(const Matrix& returns, 
                                          const Matrix& weights);
    
    static double calculateAnnualizedVolatility(const Matrix& returns, 
                                              const Matrix& weights);
    
    // Constants
    static constexpr int TRADING_DAYS_PER_YEAR = 252;
    static constexpr int PRECISION = 4;
};