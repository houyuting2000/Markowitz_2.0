#include "PortfolioOptimizer.hpp"
#include "RiskReporter.hpp"
#include "StressTesting.hpp"
#include "TransactionCostModel.hpp"
#include "PortfolioRebalancer.hpp"
#include "RiskMetrics.hpp"
#include "RiskConstraints.hpp"
#include "CSVParser.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>

using namespace QuantLib;
using namespace std;

class EnhancedPortfolioOptimizer {
private:
    // Constants
    static const int NUM_ASSETS = 12;
    static const int NUM_PERIODS = 847;
    static const int DATE_COLUMN = 1;
    static const int FIRST_ASSET_COLUMN = 2;
    static const int BENCHMARK_COLUMN = 14;
    static const int TRADING_DAYS_PER_YEAR = 252;
    static const int TRADING_DAYS_PER_MONTH = 22;

    // Core data structures
    Matrix returns_;
    Matrix excessReturns_;
    Matrix covariance_;
    Matrix excessCovariance_;
    Matrix teWeights_;
    Matrix mptWeights_;
    Matrix currentWeights_;
    Real benchmarkReturn_;
    vector<tuple<Real, Real, Real>> efficientFrontierPoints_;
    vector<string> dates_;
    int windowSize_;

    // Risk management components
    unique_ptr<RiskMetrics> riskMetrics_;
    unique_ptr<RiskConstraints> riskConstraints_;
    RiskMetrics::PortfolioRisk currentRisk_;
    map<int, string> sectorMap_;
    vector<double> averageDailyVolume_;

    // Transaction cost model
    TransactionCostModel costModel_;

    // Performance metrics
    Real dailyReturn_;
    Real monthlyReturn_;
    Real dailyVol_;
    Real monthlyVol_;
    Real trackingError_;

    // Helper methods
    Matrix calculateMarkowitzWeights(
        const Matrix& mu, 
        const Matrix& sigma, 
        const Matrix& u, 
        Real targetReturn, 
        Real& optMu, 
        Real& optSigmaSq) {
        
        try {
            Matrix weights(NUM_ASSETS, 1);
            
            // Original Markowitz calculation
            Real A = (transpose(mu)*inverse(sigma)*mu)[0][0];
            Real B = (transpose(mu)*inverse(sigma)*u)[0][0];
            Real C = (transpose(u)*inverse(sigma)*u)[0][0];
            Real D = A - B * B / C;
            
            optMu = A / C;
            optSigmaSq = 1 / C;
            weights = inverse(sigma)*u / C * (A - B * targetReturn) / D + 
                     inverse(sigma)*mu / B * (targetReturn*B - B * B / C) / D;
            
            return weights;
        }
        catch (const exception& e) {
            throw runtime_error("Error in calculateMarkowitzWeights: " + string(e.what()));
        }
    }

    void calculatePerformanceMetrics() {
        try {
            // Calculate basic metrics
            dailyReturn_ = (transpose(teWeights_)*returns_)[0][0];
            dailyVol_ = sqrt((transpose(teWeights_)*covariance_*teWeights_)[0][0]);
            trackingError_ = sqrt((transpose(teWeights_)*excessCovariance_*teWeights_)[0][0]);
            monthlyReturn_ = pow(1 + dailyReturn_, TRADING_DAYS_PER_MONTH) - 1;
            monthlyVol_ = dailyVol_ * sqrt(TRADING_DAYS_PER_MONTH);

            // Calculate comprehensive risk metrics
            Matrix benchmarkReturns(NUM_PERIODS, 1);
            for (int i = 0; i < NUM_PERIODS; i++) {
                benchmarkReturns[i][0] = stod(CSVParser::getField(i, BENCHMARK_COLUMN));
            }

            currentRisk_ = riskMetrics_->calculateRiskMetrics(
                teWeights_,
                returns_,
                covariance_,
                excessReturns_,
                excessCovariance_,
                benchmarkReturns
            );
        }
        catch (const exception& e) {
            throw runtime_error("Error in calculatePerformanceMetrics: " + string(e.what()));
        }
    }

    vector<string> extractDates(const string& filename) {
        try {
            Parser portfolio(filename);
            vector<string> dates;
            for (int i = 0; i < NUM_PERIODS; i++) {
                dates.push_back(portfolio[i][DATE_COLUMN]);
            }
            return dates;
        }
        catch (const exception& e) {
            throw runtime_error("Error extracting dates: " + string(e.what()));
        }
    }

    void updateCovariances(
        const Matrix& windowReturns, 
        const Matrix& windowExcessReturns) {
        
        try {
            SequenceStatistics ss, ssd;
            for (int i = 0; i < windowReturns.rows(); i++) {
                vector<Real> dailyReturns, excessReturns;
                for (int j = 0; j < NUM_ASSETS; j++) {
                    dailyReturns.push_back(windowReturns[i][j]);
                    excessReturns.push_back(windowExcessReturns[i][j]);
                }
                ss.add(dailyReturns);
                ssd.add(excessReturns);
            }
            covariance_ = ss.covariance();
            excessCovariance_ = ssd.covariance();
        }
        catch (const exception& e) {
            throw runtime_error("Error in updateCovariances: " + string(e.what()));
        }
    }

    void initializeSectorMap() {
        // Initialize sector mappings for risk constraints
        sectorMap_ = {
            {0, "Technology"},
            {1, "Automotive"},
            {2, "Consumer Staples"},
            {3, "International"},
            {4, "Financial Services"},
            {5, "Financial Services"},
            {6, "Technology"},
            {7, "Consumer Discretionary"},
            {8, "Industrial"},
            {9, "Consumer Discretionary"},
            {10, "Financial Services"},
            {11, "Retail"}
        };
    }

    void initializeADV() {
        // Initialize average daily volume data (in millions)
        averageDailyVolume_ = {
            10.5,  // MSFT
            8.2,   // F
            0.5,   // BGS
            1.2,   // ADRD
            5.8,   // V
            0.3,   // MGI
            7.4,   // NFLX
            0.4,   // JACK
            6.1,   // GE
            4.3,   // SBUX
            9.7,   // C
            3.9    // HD
        };
        
        // Convert to actual volume
        for (auto& vol : averageDailyVolume_) {
            vol *= 1000000.0;
        }
    }

public:
    EnhancedPortfolioOptimizer(const string& filename, int windowSize = 252) 
        : windowSize_(windowSize) {
        try {
            // Initialize risk management components
            riskMetrics_ = make_unique<RiskMetrics>(TRADING_DAYS_PER_YEAR);
            
            RiskConstraints::ConstraintLimits limits;
            limits.maxPositionSize = 0.15;      // 15% maximum position
            limits.minPositionSize = -0.05;     // 5% maximum short
            limits.maxSectorExposure = 0.25;    // 25% sector limit
            limits.maxVolatility = 0.20;        // 20% volatility cap
            limits.maxTrackingError = 0.06;     // 6% tracking error limit
            limits.maxTurnover = 0.15;          // 15% monthly turnover limit
            
            riskConstraints_ = make_unique<RiskConstraints>(limits);

            // Initialize sector map and ADV data
            initializeSectorMap();
            initializeADV();

            // Load data and initialize portfolio
            loadData(filename);
            dates_ = extractDates(filename);
            
            // Initialize transaction cost model
            TransactionCostModel::Costs costs;
            costs.fixedCommission = 0.0001;     // 1 bp per trade
            costs.variableCommission = 0.0005;  // 5 bps
            costs.marketImpact = 0.1;           // Market impact coefficient
            costs.slippage = 0.0002;           // 2 bps average slippage
            costModel_.setCosts(costs);

            // Initialize current weights to equal weight
            currentWeights_ = Matrix(NUM_ASSETS, 1, 1.0/NUM_ASSETS);
        }
        catch (const exception& e) {
            throw runtime_error("Error in constructor: " + string(e.what()));
        }
    }


    void loadData(const string& filename) {
        try {
            Parser portfolio(filename);
            returns_ = Matrix(NUM_PERIODS, NUM_ASSETS);
            excessReturns_ = Matrix(NUM_PERIODS, NUM_ASSETS);
            
            for (int i = 0; i < NUM_PERIODS; i++) {
                double benchmarkReturn = stod(portfolio[i][BENCHMARK_COLUMN]);
                for (int j = 0; j < NUM_ASSETS; j++) {
                    returns_[i][j] = stod(portfolio[i][j + FIRST_ASSET_COLUMN]);
                    excessReturns_[i][j] = returns_[i][j] - benchmarkReturn;
                }
            }
        }
        catch (const exception& e) {
            throw runtime_error("Error loading data: " + string(e.what()));
        }
    }

    void optimizePortfolio() {
        try {
            // Calculate initial optimization
            Matrix windowReturns = returns_.block(0, 0, windowSize_, NUM_ASSETS);
            Matrix windowExcessReturns = excessReturns_.block(0, 0, windowSize_, NUM_ASSETS);
            
            updateCovariances(windowReturns, windowExcessReturns);
            
            // Calculate efficient frontier points
            calculateEfficientFrontier();
            
            // Optimize tracking error
            optimizeTrackingError();
            
            // Apply risk constraints
            Matrix benchmarkReturns(NUM_PERIODS, 1);
            for (int i = 0; i < NUM_PERIODS; i++) {
                benchmarkReturns[i][0] = stod(CSVParser::getField(i, BENCHMARK_COLUMN));
            }
            
            teWeights_ = riskConstraints_->enforceConstraints(
                teWeights_,
                currentWeights_,
                returns_,
                covariance_,
                benchmarkReturns,
                sectorMap_,
                averageDailyVolume_
            );
            
            // Calculate performance metrics
            calculatePerformanceMetrics();
        }
        catch (const exception& e) {
            throw runtime_error("Error in optimizePortfolio: " + string(e.what()));
        }
    }

    void optimizeTrackingError() {
        try {
            Matrix mu(NUM_ASSETS, 1);
            Matrix u(NUM_ASSETS, 1, 1.0);
            
            // Calculate mean returns
            for (int i = 0; i < NUM_ASSETS; i++) {
                double sum = 0.0;
                for (int j = 0; j < windowSize_; j++) {
                    sum += returns_[j][i];
                }
                mu[i][0] = sum / windowSize_;
            }
            
            // Minimize tracking error
            Real optMu, optSigmaSq;
            teWeights_ = calculateMarkowitzWeights(mu, excessCovariance_, u, 0.0, optMu, optSigmaSq);
            
            // Apply transaction cost optimization
            teWeights_ = costModel_.optimizeWithCosts(
                teWeights_,
                currentWeights_,
                covariance_,
                averageDailyVolume_
            );
        }
        catch (const exception& e) {
            throw runtime_error("Error in optimizeTrackingError: " + string(e.what()));
        }
    }

    void calculateEfficientFrontier() {
        try {
            const int NUM_POINTS = 50;
            Matrix mu(NUM_ASSETS, 1);
            Matrix u(NUM_ASSETS, 1, 1.0);
            
            // Calculate mean returns
            for (int i = 0; i < NUM_ASSETS; i++) {
                double sum = 0.0;
                for (int j = 0; j < windowSize_; j++) {
                    sum += returns_[j][i];
                }
                mu[i][0] = sum / windowSize_;
            }
            
            // Calculate efficient frontier points
            efficientFrontierPoints_.clear();
            Real minRet = *min_element(mu.begin(), mu.end());
            Real maxRet = *max_element(mu.begin(), mu.end());
            Real step = (maxRet - minRet) / (NUM_POINTS - 1);
            
            for (int i = 0; i < NUM_POINTS; i++) {
                Real targetReturn = minRet + i * step;
                Real optMu, optSigmaSq;
                Matrix weights = calculateMarkowitzWeights(mu, covariance_, u, targetReturn, optMu, optSigmaSq);
                efficientFrontierPoints_.push_back(make_tuple(targetReturn, sqrt(optSigmaSq), optMu));
            }
        }
        catch (const exception& e) {
            throw runtime_error("Error in calculateEfficientFrontier: " + string(e.what()));
        }
    }

    // Reporting and analysis methods
    void generateRiskReport(const string& filename) {
        try {
            ofstream report(filename);
            report << fixed << setprecision(4);
            
            // Portfolio summary
            report << "Portfolio Risk Analysis Report\n";
            report << "==============================\n\n";
            
            // Risk metrics
            report << "Risk Metrics:\n";
            report << "--------------\n";
            report << "Daily Volatility: " << currentRisk_.dailyVol * 100 << "%\n";
            report << "Monthly Volatility: " << currentRisk_.monthlyVol * 100 << "%\n";
            report << "Annualized Volatility: " << currentRisk_.annualizedVol * 100 << "%\n";
            report << "Tracking Error: " << currentRisk_.trackingError * 100 << "%\n";
            report << "Information Ratio: " << currentRisk_.informationRatio << "\n";
            report << "Sharpe Ratio: " << currentRisk_.sharpeRatio << "\n";
            report << "Sortino Ratio: " << currentRisk_.sortino << "\n";
            report << "Maximum Drawdown: " << currentRisk_.maxDrawdown * 100 << "%\n";
            report << "Beta: " << currentRisk_.beta << "\n";
            report << "Alpha: " << currentRisk_.alpha * 100 << "%\n\n";
            
            // Position analysis
            report << "Position Analysis:\n";
            report << "-----------------\n";
            for (int i = 0; i < NUM_ASSETS; i++) {
                report << "Asset " << i + 1 << ": " << teWeights_[i][0] * 100 << "%\n";
            }
            report << "\n";
            
            // Sector exposures
            report << "Sector Exposures:\n";
            report << "----------------\n";
            map<string, double> sectorExposures;
            for (int i = 0; i < NUM_ASSETS; i++) {
                sectorExposures[sectorMap_[i]] += teWeights_[i][0];
            }
            for (const auto& exposure : sectorExposures) {
                report << exposure.first << ": " << exposure.second * 100 << "%\n";
            }
            report << "\n";
            
            // Risk constraints status
            report << "Risk Constraints Status:\n";
            report << "----------------------\n";
            auto constraintStatus = riskConstraints_->getActiveViolations();
            if (constraintStatus.empty()) {
                report << "All constraints satisfied\n";
            } else {
                for (const auto& violation : constraintStatus) {
                    report << "Violation: " << violation << "\n";
                }
            }
            report << "\n";
            
            // Transaction cost analysis
            report << "Transaction Cost Analysis:\n";
            report << "------------------------\n";
            auto costs = costModel_.calculateTotalCosts(teWeights_, currentWeights_, averageDailyVolume_);
            report << "Estimated Trading Costs: " << costs * 100 << " bps\n\n";
            
            report.close();
        }
        catch (const exception& e) {
            throw runtime_error("Error generating risk report: " + string(e.what()));
        }
    }

    // Getter methods for portfolio analysis
    Matrix getOptimizedWeights() const { return teWeights_; }
    Matrix getCurrentWeights() const { return currentWeights_; }
    RiskMetrics::PortfolioRisk getCurrentRisk() const { return currentRisk_; }
    vector<tuple<Real, Real, Real>> getEfficientFrontier() const { return efficientFrontierPoints_; }
    
    // Portfolio update method
    void updatePortfolio(const Matrix& newWeights) {
        try {
            currentWeights_ = newWeights;
            optimizePortfolio();
        }
        catch (const exception& e) {
            throw runtime_error("Error updating portfolio: " + string(e.what()));
        }
    }
};

// Main function
int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            cerr << "Usage: " << argv[0] << " <portfolio_data_file>" << endl;
            return 1;
        }

        string filename = argv[1];
        EnhancedPortfolioOptimizer optimizer(filename);
        
        // Perform initial optimization
        optimizer.optimizePortfolio();
        
        // Generate risk report
        optimizer.generateRiskReport("portfolio_risk_report.txt");
        
        // Output optimized weights
        Matrix optimizedWeights = optimizer.getOptimizedWeights();
        cout << "\nOptimized Portfolio Weights:\n";
        for (int i = 0; i < optimizedWeights.rows(); i++) {
            cout << "Asset " << i + 1 << ": " << fixed << setprecision(4) 
                 << optimizedWeights[i][0] * 100 << "%\n";
        }
        
        // Output key risk metrics
        auto risk = optimizer.getCurrentRisk();
        cout << "\nKey Risk Metrics:\n";
        cout << "Tracking Error: " << risk.trackingError * 100 << "%\n";
        cout << "Information Ratio: " << risk.informationRatio << "\n";
        cout << "Sharpe Ratio: " << risk.sharpeRatio << "\n";
        
        return 0;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}