#include "TransactionCostModel.hpp"
#include <cmath>
#include <stdexcept>

double TransactionCostModel::calculateTotalCost(
    const Matrix& currentWeights,
    const Matrix& targetWeights,
    const Matrix& prices,
    double portfolioValue) {
    
    if (currentWeights.rows() != targetWeights.rows() || 
        currentWeights.rows() != prices.rows()) {
        throw std::invalid_argument("Dimension mismatch in weights or prices");
    }

    double totalCost = 0.0;
    
    for (Size i = 0; i < currentWeights.rows(); ++i) {
        double tradeSize = std::abs(targetWeights[i][0] - currentWeights[i][0]) * 
                          portfolioValue;
        
        if (tradeSize > 0) {
            // Fixed commission per trade
            totalCost += costs_.fixedCommission;
            
            // Variable commission based on trade size
            totalCost += tradeSize * costs_.variableCommission;
            
            // Market impact cost
            double marketImpact = calculateMarketImpactDecay(
                tradeSize, avgVolumes_[i], daysToExecute_);
            totalCost += marketImpact;
            
            // Slippage cost
            totalCost += estimateSlippage(tradeSize, avgVolumes_[i]);
        }
    }
    
    return totalCost;
}

double TransactionCostModel::calculateMarketImpactDecay(
    double tradeSize, double avgVolume, int daysToExecute) {
    
    if (daysToExecute <= 0) {
        throw std::invalid_argument("Days to execute must be positive");
    }
    
    double totalImpact = 0.0;
    double remainingSize = tradeSize;
    
    for (int day = 0; day < daysToExecute; ++day) {
        // Calculate size to trade on this day
        double dailyTradeSize = remainingSize / (daysToExecute - day);
        
        // Calculate impact for this day's trading
        double dailyImpact = estimateMarketImpact(dailyTradeSize, avgVolume);
        
        // Apply decay factor
        totalImpact += dailyImpact * std::exp(-decayRate_ * day);
        
        // Update remaining size
        remainingSize -= dailyTradeSize;
    }
    
    return totalImpact;
}

double TransactionCostModel::estimateSlippage(double tradeSize, double avgVolume) {
    if (avgVolume <= 0) {
        throw std::invalid_argument("Average volume must be positive");
    }
    
    // Square root model for price impact
    return costs_.slippageModel * std::sqrt(tradeSize / avgVolume);
}

double TransactionCostModel::estimateMarketImpact(double tradeSize, double avgVolume) {
    if (avgVolume <= 0) {
        throw std::invalid_argument("Average volume must be positive");
    }
    
    // Linear component
    double linearImpact = costs_.marketImpact * (tradeSize / avgVolume);
    
    // Non-linear component (power law)
    double nonLinearImpact = costs_.marketImpact * 
                            std::pow(tradeSize / avgVolume, 1.5);
    
    return linearImpact + nonLinearImpact;
}

void TransactionCostModel::updateMarketData(
    const std::vector<double>& newVolumes,
    const std::vector<double>& newPrices) {
    
    if (newVolumes.size() != newPrices.size()) {
        throw std::invalid_argument("Volume and price data size mismatch");
    }
    
    avgVolumes_ = newVolumes;
    currentPrices_ = newPrices;
}

void TransactionCostModel::setCosts(const Costs& costs) {
    if (costs.fixedCommission < 0 || costs.variableCommission < 0 ||
        costs.slippageModel < 0 || costs.marketImpact < 0) {
        throw std::invalid_argument("Cost parameters must be non-negative");
    }
    
    costs_ = costs;
}