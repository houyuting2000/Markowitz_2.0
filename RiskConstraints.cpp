#include "RiskConstraints.hpp"
#include <numeric>
#include <cmath>
#include <stdexcept>

bool RiskConstraints::validatePortfolio(
    const Matrix& weights, 
    const std::vector<SectorExposure>& sectorExposures) {
    
    try {
        // Check position size limits
        if (!checkPositionLimits(weights)) {
            return false;
        }
        
        // Check sector exposure limits
        if (!checkSectorExposures(weights, sectorExposures)) {
            return false;
        }
        
        // Check volatility limit
        double portfolioVol = calculatePortfolioVolatility(weights);
        if (portfolioVol > limits_.maxVolatility) {
            return false;
        }
        
        // Check drawdown limit
        double maxDrawdown = calculateMaxDrawdown(weights);
        if (maxDrawdown > limits_.maxDrawdown) {
            return false;
        }
        
        // Check liquidity constraint
        if (!checkLiquidityConstraint(weights)) {
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Portfolio validation failed: " + 
                               std::string(e.what()));
    }
}

bool RiskConstraints::checkPositionLimits(const Matrix& weights) {
    double sumWeights = 0.0;
    
    for (Size i = 0; i < weights.rows(); ++i) {
        if (weights[i][0] < limits_.minPositionSize || 
            weights[i][0] > limits_.maxPositionSize) {
            return false;
        }
        sumWeights += weights[i][0];
    }
    
    // Check if weights sum to 1 (within numerical tolerance)
    return std::abs(sumWeights - 1.0) < 1e-6;
}

bool RiskConstraints::checkSectorExposures(
    const Matrix& weights,
    const std::vector<SectorExposure>& sectorExposures) {
    
    std::unordered_map<std::string, double> sectorWeights;
    
    // Calculate sector weights
    for (const auto& sector : sectorExposures) {
        double sectorWeight = 0.0;
        for (const auto& stock : sector.stocks) {
            auto it = stockIndices_.find(stock);
            if (it != stockIndices_.end()) {
                sectorWeight += weights[it->second][0];
            }
        }
        
        if (sectorWeight > limits_.maxSectorExposure) {
            return false;
        }
        sectorWeights[sector.sector] = sectorWeight;
    }
    
    return true;
}

double RiskConstraints::calculatePortfolioVolatility(const Matrix& weights) {
    if (!covariance_.rows() || !covariance_.columns()) {
        throw std::runtime_error("Covariance matrix not initialized");
    }
    return std::sqrt((transpose(weights) * covariance_ * weights)[0][0]);
}

double RiskConstraints::calculateMaxDrawdown(const Matrix& weights) {
    if (!returns_.rows() || !returns_.columns()) {
        throw std::runtime_error("Returns matrix not initialized");
    }

    double maxDrawdown = 0.0;
    double peak = 1.0;
    double value = 1.0;
    
    for (Size i = 0; i < returns_.rows(); ++i) {
        double dailyReturn = 0.0;
        for (Size j = 0; j < weights.rows(); ++j) {
            dailyReturn += returns_[i][j] * weights[j][0];
        }
        
        value *= (1 + dailyReturn);
        peak = std::max(peak, value);
        maxDrawdown = std::max(maxDrawdown, (peak - value) / peak);
    }
    
    return maxDrawdown;
}

bool RiskConstraints::checkLiquidityConstraint(const Matrix& weights) {
    if (avgVolumes_.empty()) {
        throw std::runtime_error("Average volumes not initialized");
    }

    for (Size i = 0; i < weights.rows(); ++i) {
        double position = weights[i][0] * portfolioValue_;
        if (position > avgVolumes_[i] * limits_.minLiquidity) {
            return false;
        }
    }
    return true;
}

void RiskConstraints::updateMarketData(
    const Matrix& returns,
    const Matrix& covariance,
    const std::vector<double>& volumes) {
    
    if (returns.rows() == 0 || covariance.rows() == 0 || volumes.empty()) {
        throw std::invalid_argument("Invalid market data provided");
    }

    returns_ = returns;
    covariance_ = covariance;
    avgVolumes_ = volumes;
}