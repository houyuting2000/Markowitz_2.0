#pragma once
#include <ql/quantlib.hpp>
#include <string>
#include <vector>
#include <unordered_map>

using namespace QuantLib;

class RiskConstraints {
public:
    struct Limits {
        double maxPositionSize{0.2};
        double minPositionSize{0.0};
        double maxSectorExposure{0.3};
        double maxVolatility{0.25};
        double maxDrawdown{0.15};
        double minLiquidity{1000000}; // Min daily trading volume
    };

    struct SectorExposure {
        std::string sector;
        double exposure;
        std::vector<std::string> stocks;
    };

    // Constructor
    RiskConstraints() = default;

    // Main validation method
    bool validatePortfolio(const Matrix& weights, 
                          const std::vector<SectorExposure>& sectorExposures);

    // Setters
    void setPortfolioValue(double value) { portfolioValue_ = value; }
    void setStockIndices(const std::unordered_map<std::string, size_t>& indices) {
        stockIndices_ = indices;
    }
    void setLimits(const Limits& limits) { limits_ = limits; }
    
    // Market data update
    void updateMarketData(const Matrix& returns,
                         const Matrix& covariance,
                         const std::vector<double>& volumes);

private:
    // Member variables
    Limits limits_;
    double portfolioValue_{0.0};
    std::unordered_map<std::string, size_t> stockIndices_;
    std::vector<double> avgVolumes_;
    Matrix returns_;
    Matrix covariance_;
    
    // Private validation methods
    bool checkPositionLimits(const Matrix& weights);
    bool checkSectorExposures(const Matrix& weights,
                            const std::vector<SectorExposure>& sectorExposures);
    bool checkLiquidityConstraint(const Matrix& weights);
    
    // Risk calculation methods
    double calculatePortfolioVolatility(const Matrix& weights);
    double calculateMaxDrawdown(const Matrix& weights);
};