#pragma once

#include <ql/quantlib.hpp>
#include <memory>
#include <random>
#include "DataManager.hpp"
#include "RiskConstraints.hpp"
#include "TransactionCostModel.hpp"

using namespace QuantLib;

class PortfolioOptimizer {
private:
    struct OptimizationParameters {
        double riskAversion{3.0};
        double targetReturn{0.10};
        int maxIterations{1000};
        double convergenceTolerance{1e-8};
        bool useTransactionCosts{true};
        bool useSectorConstraints{true};
        double maxTradingCost{0.01}; // 1% max trading cost
    };

    std::unique_ptr<DataManager> dataManager_;
    std::unique_ptr<RiskConstraints> riskConstraints_;
    std::unique_ptr<TransactionCostModel> costModel_;
    OptimizationParameters params_;

    // Private helper methods
    bool checkConvergence(const Matrix& oldWeights, 
                         const Matrix& newWeights,
                         double tolerance);
    Matrix generateCandidateWeights(const Matrix& currentWeights);
    bool isImprovement(double newReturn, double newRisk,
                      double currentReturn, double currentRisk);
    double calculatePortfolioReturn(const Matrix& weights);
    double calculatePortfolioRisk(const Matrix& weights);
    std::vector<RiskConstraints::SectorExposure> getSectorExposures() const;
    Matrix getPrices() const;

public:
    // Constructor
    PortfolioOptimizer(std::unique_ptr<DataManager> dataManager,
                      std::unique_ptr<RiskConstraints> riskConstraints,
                      std::unique_ptr<TransactionCostModel> costModel)
        : dataManager_(std::move(dataManager))
        , riskConstraints_(std::move(riskConstraints))
        , costModel_(std::move(costModel)) {}

    // Main optimization methods
    Matrix optimizeWithConstraints(const Matrix& currentWeights,
                                 double portfolioValue,
                                 const OptimizationParameters& params = {});

    Matrix generateTradeList(const Matrix& currentWeights,
                           const Matrix& targetWeights,
                           double portfolioValue);

    // Setters
    void setOptimizationParameters(const OptimizationParameters& params) {
        params_ = params;
    }
};