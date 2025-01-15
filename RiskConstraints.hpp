#pragma once
#include <ql/quantlib.hpp>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <stdexcept>

using namespace QuantLib;

class RiskConstraints {
public:
    struct ConstraintLimits {
        // Position limits
        double maxPositionSize{0.2};      // Maximum weight for any single position
        double minPositionSize{-0.1};     // Minimum weight (allows shorts)
        double maxShortPosition{0.3};     // Maximum total short exposure
        
        // Sector/Factor limits
        double maxSectorExposure{0.3};    // Maximum exposure to any sector
        double maxFactorExposure{0.2};    // Maximum exposure to any factor
        double maxBetaDeviation{0.2};     // Maximum deviation from benchmark beta
        
        // Risk limits
        double maxVolatility{0.15};       // Maximum portfolio volatility
        double maxTrackingError{0.05};    // Maximum tracking error
        double minSharpeRatio{0.5};       // Minimum Sharpe ratio
        
        // Trading limits
        double maxTurnover{0.2};          // Maximum monthly turnover
        double minTradeSize{0.001};       // Minimum trade size
        double maxTradeSize{0.05};        // Maximum single trade size
        
        // Liquidity constraints
        double minLiquidity{1000000.0};   // Minimum daily liquidity
        double maxADVPercent{0.1};        // Maximum % of average daily volume
        
        // Diversification
        int minPositions{10};             // Minimum number of positions
        int maxPositions{50};             // Maximum number of positions
        
        ConstraintLimits() = default;
    };

    struct ConstraintStatus {
        bool positionLimitsOK{true};
        bool sectorLimitsOK{true};
        bool riskLimitsOK{true};
        bool tradingLimitsOK{true};
        bool liquidityLimitsOK{true};
        bool diversificationOK{true};
        std::vector<std::string> violations;
        
        bool allConstraintsMet() const {
            return positionLimitsOK && sectorLimitsOK && riskLimitsOK && 
                   tradingLimitsOK && liquidityLimitsOK && diversificationOK;
        }
        
        void addViolation(const std::string& violation) {
            violations.push_back(violation);
        }
        
        ConstraintStatus() = default;
    };

    explicit RiskConstraints(const ConstraintLimits& limits = ConstraintLimits());
    ~RiskConstraints() = default;

    // Main constraint checking methods
    ConstraintStatus checkAllConstraints(
        const Matrix& proposedWeights,
        const Matrix& currentWeights,
        const Matrix& returns,
        const Matrix& covariance,
        const Matrix& benchmarkReturns,
        const std::map<int, std::string>& sectorMap,
        const std::vector<double>& adv);

    // Individual constraint checks
    bool checkPositionLimits(const Matrix& weights);
    bool checkSectorExposure(
        const Matrix& weights,
        const std::map<int, std::string>& sectorMap);
    bool checkVolatilityLimit(
        const Matrix& weights,
        const Matrix& covariance);
    bool checkTrackingError(
        const Matrix& weights,
        const Matrix& excessCovariance);
    bool checkTurnover(
        const Matrix& oldWeights,
        const Matrix& newWeights);
    bool checkLiquidity(
        const Matrix& weights,
        const std::vector<double>& adv);
    bool checkDiversification(const Matrix& weights);
    bool checkBetaDeviation(
        const Matrix& weights,
        const Matrix& returns,
        const Matrix& benchmarkReturns);

    // Constraint enforcement methods
    Matrix enforceConstraints(
        Matrix proposedWeights,
        const Matrix& currentWeights,
        const Matrix& returns,
        const Matrix& covariance,
        const Matrix& benchmarkReturns,
        const std::map<int, std::string>& sectorMap,
        const std::vector<double>& adv);

    // Helper methods for constraint adjustment
    Matrix adjustPositionSizes(Matrix weights);
    Matrix adjustSectorExposures(
        Matrix weights,
        const std::map<int, std::string>& sectorMap);
    Matrix adjustForVolatility(
        Matrix weights,
        const Matrix& covariance);
    Matrix adjustForLiquidity(
        Matrix weights,
        const std::vector<double>& adv);

    // Utility methods
    void setConstraintLimits(const ConstraintLimits& limits) { limits_ = limits; }
    ConstraintLimits getConstraintLimits() const { return limits_; }
    std::vector<std::string> getActiveViolations() const { return lastStatus_.violations; }
    bool areConstraintsSatisfied() const { return lastStatus_.allConstraintsMet(); }

private:
    ConstraintLimits limits_;
    ConstraintStatus lastStatus_;

    // Helper methods
    double calculateTotalShortExposure(const Matrix& weights);
    std::map<std::string, double> calculateSectorExposures(
        const Matrix& weights,
        const std::map<int, std::string>& sectorMap);
    double calculatePortfolioTurnover(
        const Matrix& oldWeights,
        const Matrix& newWeights);
    int countActivePositions(const Matrix& weights);
    void updateConstraintStatus(
        bool conditionMet,
        const std::string& violationMessage,
        bool& statusFlag);
};