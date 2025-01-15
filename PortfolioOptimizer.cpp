#include "PortfolioOptimizer.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>

Matrix PortfolioOptimizer::optimizeWithConstraints(
    const Matrix& currentWeights,
    double portfolioValue,
    const OptimizationParameters& params) {
    
    try {
        // Initialize optimization
        Matrix optimalWeights = currentWeights;
        params_ = params;
        
        // Calculate initial metrics
        double currentReturn = calculatePortfolioReturn(currentWeights);
        double currentRisk = calculatePortfolioRisk(currentWeights);
        
        // Optimization loop
        for (int iter = 0; iter < params.maxIterations; ++iter) {
            bool constraintsViolated = false;
            
            // Generate candidate weights
            Matrix candidateWeights = generateCandidateWeights(optimalWeights);
            
            // Check constraints
            if (params.useSectorConstraints && 
                !riskConstraints_->validatePortfolio(candidateWeights, getSectorExposures())) {
                constraintsViolated = true;
            }
            
            // Check transaction costs if enabled
            if (params.useTransactionCosts) {
                double tradingCost = costModel_->calculateTotalCost(
                    currentWeights, candidateWeights, getPrices(), portfolioValue);
                
                if (tradingCost > params.maxTradingCost) {
                    constraintsViolated = true;
                }
            }
            
            // Update weights if constraints are satisfied
            if (!constraintsViolated) {
                double candidateReturn = calculatePortfolioReturn(candidateWeights);
                double candidateRisk = calculatePortfolioRisk(candidateWeights);
                
                if (isImprovement(candidateReturn, candidateRisk, 
                                currentReturn, currentRisk)) {
                    optimalWeights = candidateWeights;
                    currentReturn = candidateReturn;
                    currentRisk = candidateRisk;
                }
            }
            
            // Check convergence
            if (checkConvergence(currentWeights, optimalWeights, 
                               params.convergenceTolerance)) {
                break;
            }
        }
        
        return optimalWeights;
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Optimization failed: " + std::string(e.what()));
    }
}

Matrix PortfolioOptimizer::generateTradeList(
    const Matrix& currentWeights,
    const Matrix& targetWeights,
    double portfolioValue) {
    
    Matrix trades(currentWeights.rows(), 3); // [asset_index, trade_size, direction]
    
    for (Size i = 0; i < currentWeights.rows(); ++i) {
        double difference = targetWeights[i][0] - currentWeights[i][0];
        if (std::abs(difference) > params_.convergenceTolerance) {
            trades[i][0] = i;  // asset index
            trades[i][1] = std::abs(difference) * portfolioValue;  // trade size
            trades[i][2] = difference > 0 ? 1 : -1;  // direction (buy/sell)
        }
    }
    
    return trades;
}

bool PortfolioOptimizer::checkConvergence(
    const Matrix& oldWeights,
    const Matrix& newWeights,
    double tolerance) {
    
    double maxDiff = 0.0;
    for (Size i = 0; i < oldWeights.rows(); ++i) {
        maxDiff = std::max(maxDiff, 
                          std::abs(oldWeights[i][0] - newWeights[i][0]));
    }
    
    return maxDiff < tolerance;
}

Matrix PortfolioOptimizer::generateCandidateWeights(const Matrix& currentWeights) {
    Matrix candidate = currentWeights;
    
    // Generate random perturbation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(0, 0.01);
    
    // Apply perturbation
    for (Size i = 0; i < candidate.rows(); ++i) {
        candidate[i][0] += d(gen);
    }
    
    // Normalize weights
    double sum = 0.0;
    for (Size i = 0; i < candidate.rows(); ++i) {
        candidate[i][0] = std::max(0.0, candidate[i][0]);
        sum += candidate[i][0];
    }
    
    for (Size i = 0; i < candidate.rows(); ++i) {
        candidate[i][0] /= sum;
    }
    
    return candidate;
}

bool PortfolioOptimizer::isImprovement(
    double newReturn, double newRisk,
    double currentReturn, double currentRisk) {
    
    // Calculate utility using risk aversion parameter
    double newUtility = newReturn - params_.riskAversion * newRisk;
    double currentUtility = currentReturn - params_.riskAversion * currentRisk;
    
    return newUtility > currentUtility;
}

double PortfolioOptimizer::calculatePortfolioReturn(const Matrix& weights) {
    Matrix returns = dataManager_->getReturns();
    return (transpose(weights) * returns)[0][0];
}

double PortfolioOptimizer::calculatePortfolioRisk(const Matrix& weights) {
    Matrix covariance = dataManager_->getCovarianceMatrix();
    return std::sqrt((transpose(weights) * covariance * weights)[0][0]);
}

std::vector<RiskConstraints::SectorExposure> 
PortfolioOptimizer::getSectorExposures() const {
    return dataManager_->getSectorExposures();
}

Matrix PortfolioOptimizer::getPrices() const {
    return dataManager_->getPrices();
}