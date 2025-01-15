#include "PortfolioRebalancer.hpp"

void PortfolioRebalancer::updateRebalancingDates(const vector<string>& allDates) {
    // Extract month-end dates
    string currentMonth = "";
    for (const auto& date : allDates) {
        string month = date.substr(0, date.find_last_of('/'));
        if (month != currentMonth) {
            rebalanceDates_.push_back(date);
            currentMonth = month;
        }
    }
}

Real PortfolioRebalancer::calculateTurnover(const Matrix& oldWeights, 
                                          const Matrix& newWeights) {
    Real turnover = 0.0;
    for (int i = 0; i < oldWeights.rows(); ++i) {
        turnover += std::abs(newWeights[i][0] - oldWeights[i][0]);
    }
    return turnover / 2.0;
}

void PortfolioRebalancer::rebalance(const string& currentDate) {
    // Check if rebalancing is needed
    if (std::find(rebalanceDates_.begin(), rebalanceDates_.end(), 
                  currentDate) == rebalanceDates_.end()) {
        return;  // Not a rebalancing date
    }

    // Store old weights for turnover calculation
    Matrix oldWeights = currentWeights_;

    // Get new optimal weights
    optimizer_.optimizePortfolio(currentPeriod_);
    Matrix newWeights = optimizer_.getOptimalWeights();

    // Calculate turnover and transaction costs
    Real turnover = calculateTurnover(oldWeights, newWeights);
    Real transactionCosts = costModel_.calculateCosts(turnover);

    // Apply new weights if beneficial
    if (transactionCosts < optimizer_.getExpectedExcessReturn()) {
        currentWeights_ = newWeights;
    }

    currentPeriod_++;
} 