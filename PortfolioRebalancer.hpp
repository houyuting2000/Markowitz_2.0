#pragma once
#include "PortfolioOptimizer.hpp"
#include "TransactionCostModel.hpp"
#include <vector>
#include <string>

class PortfolioRebalancer {
private:
    static const int DAYS_PER_MONTH = 22;  // Trading days
    EnhancedPortfolioOptimizer& optimizer_;
    TransactionCostModel costModel_;
    
    Matrix currentWeights_;
    vector<string> rebalanceDates_;
    int currentPeriod_;

    void updateRebalancingDates(const vector<string>& allDates);
    Real calculateTurnover(const Matrix& oldWeights, const Matrix& newWeights);

public:
    PortfolioRebalancer(EnhancedPortfolioOptimizer& optimizer) 
        : optimizer_(optimizer), currentPeriod_(0) {}
    
    void initialize(const Matrix& initialWeights);
    void rebalance(const string& currentDate);
    Matrix getCurrentWeights() const { return currentWeights_; }
}; 