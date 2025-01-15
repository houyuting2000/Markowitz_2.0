#pragma once
#include <ql/quantlib.hpp>
#include <vector>

using namespace QuantLib;

class TransactionCostModel {
public:
    struct Costs {
        double fixedCommission{0.0};      // Fixed commission per trade
        double variableCommission{0.0};    // Variable commission rate
        double slippageModel{0.0};        // Slippage model parameter
        double marketImpact{0.0};         // Market impact parameter
    };

    // Constructors
    TransactionCostModel(int daysToExecute = 1, double decayRate = 0.1) 
        : daysToExecute_(daysToExecute)
        , decayRate_(decayRate) {}

    // Main cost calculation methods
    double calculateTotalCost(const Matrix& currentWeights,
                            const Matrix& targetWeights,
                            const Matrix& prices,
                            double portfolioValue);

    // New helper method for rebalancing
    double estimateRebalancingCosts(const Matrix& oldWeights, 
                                   const Matrix& newWeights,
                                   double portfolioValue);

    // Market data update methods
    void updateMarketData(const std::vector<double>& newVolumes,
                         const std::vector<double>& newPrices);

    // Setters and getters
    void setCosts(const Costs& costs) { costs_ = costs; }
    const Costs& getCosts() const { return costs_; }
    void setDaysToExecute(int days) { daysToExecute_ = days; }
    void setDecayRate(double rate) { decayRate_ = rate; }
    
    // New methods for cost analysis
    double calculateTurnover(const Matrix& oldWeights, const Matrix& newWeights);
    double estimateMarketImpact(double tradeSize, double avgVolume);
    double estimateSlippage(double tradeSize, double avgVolume);

private:
    // Member variables
    Costs costs_;
    std::vector<double> avgVolumes_;
    std::vector<double> currentPrices_;
    int daysToExecute_;
    double decayRate_;

    // Helper methods
    double calculateMarketImpactDecay(double tradeSize, 
                                    double avgVolume, 
                                    int daysToExecute);
};