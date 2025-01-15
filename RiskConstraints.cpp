#include "RiskConstraints.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>

RiskConstraints::RiskConstraints(const ConstraintLimits& limits)
    : limits_(limits), lastStatus_() {}

RiskConstraints::ConstraintStatus RiskConstraints::checkAllConstraints(
    const Matrix& proposedWeights,
    const Matrix& currentWeights,
    const Matrix& returns,
    const Matrix& covariance,
    const Matrix& benchmarkReturns,
    const std::map<int, std::string>& sectorMap,
    const std::vector<double>& adv) {
    
    lastStatus_ = ConstraintStatus();  // Reset status

    // Check position limits
    updateConstraintStatus(
        checkPositionLimits(proposedWeights),
        "Position size limits violated",
        lastStatus_.positionLimitsOK
    );

    // Check sector exposure
    updateConstraintStatus(
        checkSectorExposure(proposedWeights, sectorMap),
        "Sector exposure limits violated",
        lastStatus_.sectorLimitsOK
    );

    // Check risk limits
    bool riskLimitsOK = checkVolatilityLimit(proposedWeights, covariance) &&
                        checkBetaDeviation(proposedWeights, returns, benchmarkReturns);
    updateConstraintStatus(
        riskLimitsOK,
        "Risk limits violated",
        lastStatus_.riskLimitsOK
    );

    // Check trading limits
    updateConstraintStatus(
        checkTurnover(currentWeights, proposedWeights),
        "Turnover limits violated",
        lastStatus_.tradingLimitsOK
    );

    // Check liquidity limits
    updateConstraintStatus(
        checkLiquidity(proposedWeights, adv),
        "Liquidity constraints violated",
        lastStatus_.liquidityLimitsOK
    );

    // Check diversification
    updateConstraintStatus(
        checkDiversification(proposedWeights),
        "Diversification requirements not met",
        lastStatus_.diversificationOK
    );

    return lastStatus_;
}

bool RiskConstraints::checkPositionLimits(const Matrix& weights) {
    try {
        double totalShort = calculateTotalShortExposure(weights);
        
        // Check individual position limits
        for (int i = 0; i < weights.rows(); ++i) {
            if (weights[i][0] > limits_.maxPositionSize ||
                weights[i][0] < limits_.minPositionSize) {
                return false;
            }
        }
        
        // Check total short exposure
        if (totalShort > limits_.maxShortPosition) {
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkPositionLimits: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkSectorExposure(
    const Matrix& weights,
    const std::map<int, std::string>& sectorMap) {
    
    try {
        auto sectorExposures = calculateSectorExposures(weights, sectorMap);
        
        for (const auto& exposure : sectorExposures) {
            if (std::abs(exposure.second) > limits_.maxSectorExposure) {
                return false;
            }
        }
        
        return true;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkSectorExposure: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkVolatilityLimit(
    const Matrix& weights,
    const Matrix& covariance) {
    
    try {
        double portfolioVol = sqrt((transpose(weights)*covariance*weights)[0][0]);
        return portfolioVol <= limits_.maxVolatility;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkVolatilityLimit: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkTrackingError(
    const Matrix& weights,
    const Matrix& excessCovariance) {
    
    try {
        double trackingError = sqrt((transpose(weights)*excessCovariance*weights)[0][0]);
        return trackingError <= limits_.maxTrackingError;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkTrackingError: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkTurnover(
    const Matrix& oldWeights,
    const Matrix& newWeights) {
    
    try {
        double turnover = calculatePortfolioTurnover(oldWeights, newWeights);
        return turnover <= limits_.maxTurnover;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkTurnover: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkLiquidity(
    const Matrix& weights,
    const std::vector<double>& adv) {
    
    try {
        for (int i = 0; i < weights.rows(); ++i) {
            double position = std::abs(weights[i][0]);
            if (position * limits_.minLiquidity > adv[i] * limits_.maxADVPercent) {
                return false;
            }
        }
        return true;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkLiquidity: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkDiversification(const Matrix& weights) {
    try {
        int activePositions = countActivePositions(weights);
        return (activePositions >= limits_.minPositions &&
                activePositions <= limits_.maxPositions);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkDiversification: " + std::string(e.what()));
    }
}

bool RiskConstraints::checkBetaDeviation(
    const Matrix& weights,
    const Matrix& returns,
    const Matrix& benchmarkReturns) {
    
    try {
        // Calculate portfolio beta
        Matrix portfolioReturns = returns * weights;
        double covar = 0.0, benchmarkVar = 0.0;
        
        for (int i = 0; i < returns.rows(); ++i) {
            covar += portfolioReturns[i][0] * benchmarkReturns[i][0];
            benchmarkVar += benchmarkReturns[i][0] * benchmarkReturns[i][0];
        }
        
        double beta = covar / benchmarkVar;
        return std::abs(beta - 1.0) <= limits_.maxBetaDeviation;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in checkBetaDeviation: " + std::string(e.what()));
    }
}

Matrix RiskConstraints::enforceConstraints(
    Matrix proposedWeights,
    const Matrix& currentWeights,
    const Matrix& returns,
    const Matrix& covariance,
    const Matrix& benchmarkReturns,
    const std::map<int, std::string>& sectorMap,
    const std::vector<double>& adv) {
    
    try {
        int maxIterations = 100;
        int iteration = 0;
        bool constraintsSatisfied = false;
        
        while (!constraintsSatisfied && iteration < maxIterations) {
            // Adjust position sizes
            proposedWeights = adjustPositionSizes(proposedWeights);
            
            // Adjust sector exposures
            proposedWeights = adjustSectorExposures(proposedWeights, sectorMap);
            
            // Adjust for volatility
            proposedWeights = adjustForVolatility(proposedWeights, covariance);
            
            // Adjust for liquidity
            proposedWeights = adjustForLiquidity(proposedWeights, adv);
            
            // Check if all constraints are satisfied
            auto status = checkAllConstraints(
                proposedWeights, currentWeights, returns, covariance,
                benchmarkReturns, sectorMap, adv
            );
            
            constraintsSatisfied = status.allConstraintsMet();
            iteration++;
        }
        
        if (!constraintsSatisfied) {
            throw std::runtime_error("Failed to satisfy all constraints after maximum iterations");
        }
        
        return proposedWeights;
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in enforceConstraints: " + std::string(e.what()));
    }
}

Matrix RiskConstraints::adjustPositionSizes(Matrix weights) {
    for (int i = 0; i < weights.rows(); ++i) {
        weights[i][0] = std::max(limits_.minPositionSize,
                        std::min(limits_.maxPositionSize, weights[i][0]));
    }
    return weights;
}

Matrix RiskConstraints::adjustSectorExposures(
    Matrix weights,
    const std::map<int, std::string>& sectorMap) {
    
    auto sectorExposures = calculateSectorExposures(weights, sectorMap);
    
    for (const auto& exposure : sectorExposures) {
        if (std::abs(exposure.second) > limits_.maxSectorExposure) {
            double scaleFactor = limits_.maxSectorExposure / std::abs(exposure.second);
            
            // Scale down weights in the overexposed sector
            for (int i = 0; i < weights.rows(); ++i) {
                if (sectorMap.at(i) == exposure.first) {
                    weights[i][0] *= scaleFactor;
                }
            }
        }
    }
    
    return weights;
}

Matrix RiskConstraints::adjustForVolatility(
    Matrix weights,
    const Matrix& covariance) {
    
    double portfolioVol = sqrt((transpose(weights)*covariance*weights)[0][0]);
    
    if (portfolioVol > limits_.maxVolatility) {
        double scaleFactor = limits_.maxVolatility / portfolioVol;
        for (int i = 0; i < weights.rows(); ++i) {
            weights[i][0] *= scaleFactor;
        }
    }
    
    return weights;
}

Matrix RiskConstraints::adjustForLiquidity(
    Matrix weights,
    const std::vector<double>& adv) {
    
    for (int i = 0; i < weights.rows(); ++i) {
        double maxPosition = adv[i] * limits_.maxADVPercent / limits_.minLiquidity;
        if (std::abs(weights[i][0]) > maxPosition) {
            weights[i][0] = (weights[i][0] > 0) ? maxPosition : -maxPosition;
        }
    }
    
    return weights;
}

double RiskConstraints::calculateTotalShortExposure(const Matrix& weights) {
    double totalShort = 0.0;
    for (int i = 0; i < weights.rows(); ++i) {
        if (weights[i][0] < 0) {
            totalShort += std::abs(weights[i][0]);
        }
    }
    return totalShort;
}

std::map<std::string, double> RiskConstraints::calculateSectorExposures(
    const Matrix& weights,
    const std::map<int, std::string>& sectorMap) {
    
    std::map<std::string, double> sectorExposures;
    
    for (int i = 0; i < weights.rows(); ++i) {
        const std::string& sector = sectorMap.at(i);
        sectorExposures[sector] += weights[i][0];
    }
    
    return sectorExposures;
}

double RiskConstraints::calculatePortfolioTurnover(
    const Matrix& oldWeights,
    const Matrix& newWeights) {
    
    double turnover = 0.0;
    for (int i = 0; i < oldWeights.rows(); ++i) {
        turnover += std::abs(newWeights[i][0] - oldWeights[i][0]);
    }
    return turnover / 2.0;  // One-way turnover
}

int RiskConstraints::countActivePositions(const Matrix& weights) {
    int count = 0;
    for (int i = 0; i < weights.rows(); ++i) {
        if (std::abs(weights[i][0]) > limits_.minTradeSize) {
            count++;
        }
    }
    return count;
}

void RiskConstraints::updateConstraintStatus(
    bool conditionMet,
    const std::string& violationMessage,
    bool& statusFlag) {
    
    if (!conditionMet) {
        statusFlag = false;
        lastStatus_.addViolation(violationMessage);
    }
}