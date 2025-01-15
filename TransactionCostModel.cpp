#include "TransactionCostModel.hpp"
#include <cmath>

double TransactionCostModel::calculateTotalCost(
    const Matrix& currentWeights,
    const Matrix& targetWeights,
    const Matrix& prices,
    double portfolioValue) {
    
    double totalCost = 0.0;
    int numAssets = currentWeights.rows();

    for (int i = 0; i < numAssets; ++i) {
        double tradeSize = std::abs(targetWeights[i][0] - currentWeights[i][0]) * portfolioValue;
        
        if (tradeSize > 0) {
            // Fixed commission per trade
            totalCost += costs_.fixedCommission;
            
            // Variable commission
            totalCost += tradeSize * costs_.variableCommission;
            
            // Market impact with decay
            double marketImpact = calculateMarketImpactDecay(
                tradeSize, avgVolumes_[i], daysToExecute_);
            totalCost += marketImpact;
            
            // Slippage
            totalCost += estimateSlippage(tradeSize, avgVolumes_[i]);
        }
    }
    
    return totalCost;
}

double TransactionCostModel::estimateRebalancingCosts(
    const Matrix& oldWeights, 
    const Matrix& newWeights,
    double portfolioValue) {
    
    double turnover = calculateTurnover(oldWeights, newWeights);
    double totalCost = 0.0;
    
    // Base costs
    totalCost += costs_.fixedCommission * (turnover > 0 ? 1 : 0);  // Only if there's any trade
    totalCost += turnover * portfolioValue * costs_.variableCommission;
    
    // Market impact and slippage for each asset
    for (int i = 0; i < oldWeights.rows(); ++i) {
        double tradeSize = std::abs(newWeights[i][0] - oldWeights[i][0]) * portfolioValue;
        if (tradeSize > 0 && i < avgVolumes_.size()) {
            totalCost += calculateMarketImpactDecay(tradeSize, avgVolumes_[i], daysToExecute_);
            totalCost += estimateSlippage(tradeSize, avgVolumes_[i]);
        }
    }
    
    return totalCost;
}

void TransactionCostModel::updateMarketData(
    const std::vector<double>& newVolumes,
    const std::vector<double>& newPrices) {
    
    avgVolumes_ = newVolumes;
    currentPrices_ = newPrices;
}

double TransactionCostModel::calculateTurnover(
    const Matrix& oldWeights, 
    const Matrix& newWeights) {
    
    double turnover = 0.0;
    for (int i = 0; i < oldWeights.rows(); ++i) {
        turnover += std::abs(newWeights[i][0] - oldWeights[i][0]);
    }
    return turnover / 2.0;  // One-way turnover
}

double TransactionCostModel::estimateMarketImpact(
    double tradeSize, 
    double avgVolume) {
    
    if (avgVolume <= 0) return 0.0;
    return costs_.marketImpact * std::pow(tradeSize / avgVolume, 1.5);
}

double TransactionCostModel::estimateSlippage(
    double tradeSize, 
    double avgVolume) {
    
    if (avgVolume <= 0) return 0.0;
    return costs_.slippageModel * (tradeSize / avgVolume);
}

double TransactionCostModel::calculateMarketImpactDecay(
    double tradeSize, 
    double avgVolume, 
    int daysToExecute) {
    
    double dailyTradeSize = tradeSize / daysToExecute;
    double impact = 0.0;
    
    for (int day = 0; day < daysToExecute; ++day) {
        double decayFactor = std::exp(-decayRate_ * day);
        impact += estimateMarketImpact(dailyTradeSize, avgVolume) * decayFactor;
    }
    
    return impact;
}