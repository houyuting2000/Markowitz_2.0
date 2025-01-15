#include "PortfolioOptimizer.hpp"
#include "RiskReporter.hpp"
#include "StressTesting.hpp"
#include "TransactionCostModel.hpp"
#include "CSVParser.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <stdexcept>

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
    Real benchmarkReturn_;
    vector<tuple<Real, Real, Real>> efficientFrontierPoints_;

    // Performance metrics
    Real dailyReturn_;
    Real monthlyReturn_;
    Real dailyVol_;
    Real monthlyVol_;
    Real trackingError_;

    // Helper methods
    Matrix calculateMarkowitzWeights(const Matrix& mu, const Matrix& sigma, 
                                   const Matrix& u, Real targetReturn, 
                                   Real& optMu, Real& optSigmaSq) {
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

    void calculatePerformanceMetrics() {
        // Calculate daily metrics
        dailyReturn_ = (transpose(teWeights_)*returns_)[0][0];
        dailyVol_ = sqrt((transpose(teWeights_)*covariance_*teWeights_)[0][0]);
        trackingError_ = sqrt((transpose(teWeights_)*excessCovariance_*teWeights_)[0][0]);
        
        // Calculate monthly metrics
        monthlyReturn_ = pow(1 + dailyReturn_, TRADING_DAYS_PER_MONTH) - 1;
        monthlyVol_ = dailyVol_ * sqrt(TRADING_DAYS_PER_MONTH);
    }

public:
    EnhancedPortfolioOptimizer(const string& dataFile) {
        loadData(dataFile);
    }
    
    void loadData(const string& filename) {
        try {
            Parser portfolio(filename);
            
            // Initialize matrices
            returns_ = Matrix(NUM_PERIODS, NUM_ASSETS);
            excessReturns_ = Matrix(NUM_PERIODS, NUM_ASSETS);
            benchmarkReturn_ = 0.0;
            
            // Load returns and calculate excess returns
            for (int i = 0; i < NUM_PERIODS; i++) {
                for (int j = 0; j < NUM_ASSETS; j++) {
                    returns_[i][j] = stod(portfolio[i][j + FIRST_ASSET_COLUMN]);
                    excessReturns_[i][j] = returns_[i][j] - 
                                          stod(portfolio[i][BENCHMARK_COLUMN]);
                }
                benchmarkReturn_ += stod(portfolio[i][BENCHMARK_COLUMN]);
            }
            benchmarkReturn_ /= NUM_PERIODS;
            
            // Calculate covariance matrices
            SequenceStatistics ss, ssd;
            for (int i = 0; i < NUM_PERIODS; i++) {
                vector<Real> dailyReturns, excessReturns;
                for (int j = 0; j < NUM_ASSETS; j++) {
                    dailyReturns.push_back(returns_[i][j]);
                    excessReturns.push_back(excessReturns_[i][j]);
                }
                ss.add(dailyReturns);
                ssd.add(excessReturns);
            }
            covariance_ = ss.covariance();
            excessCovariance_ = ssd.covariance();
            
        } catch (const exception& e) {
            throw runtime_error("Error loading data: " + string(e.what()));
        }
    }

    void optimizePortfolio() {
        // Initialize parameters
        Real targetReturn = 0.0013;  // Target daily return
        Matrix u(NUM_ASSETS, 1, 1.0);  // Unit vector
        Real optMu = 0.0, optSigmaSq = 0.001;
        
        // 1. Tracking Error Optimization
        teWeights_ = calculateMarkowitzWeights(
            excessReturns_, excessCovariance_, u, targetReturn, optMu, optSigmaSq);
        
        // 2. Traditional MPT Optimization
        mptWeights_ = calculateMarkowitzWeights(
            returns_, covariance_, u, targetReturn + benchmarkReturn_, 
            optMu, optSigmaSq);
        
        // 3. Calculate performance metrics
        calculatePerformanceMetrics();
        
        // 4. Generate efficient frontier
        generateEfficientFrontier();
    }

    void generateEfficientFrontier() {
        int numPoints = 60;
        Real startReturn = -0.001;
        Real returnStep = 0.00005;
        Matrix u(NUM_ASSETS, 1, 1.0);
        Real optMu, optSigmaSq;
        
        efficientFrontierPoints_.clear();
        
        for (int i = 0; i < numPoints; i++) {
            Real targetReturn = startReturn + i * returnStep;
            
            // Calculate tracking error frontier
            Matrix teWeights = calculateMarkowitzWeights(
                excessReturns_, excessCovariance_, u, targetReturn, 
                optMu, optSigmaSq);
            Real trackingError = sqrt((transpose(teWeights)*
                                    excessCovariance_*teWeights)[0][0]);
            
            // Calculate traditional frontier
            Matrix mptWeights = calculateMarkowitzWeights(
                returns_, covariance_, u, targetReturn, optMu, optSigmaSq);
            Real portfolioVol = sqrt((transpose(mptWeights)*
                                   covariance_*mptWeights)[0][0]);
            
            efficientFrontierPoints_.push_back(
                make_tuple(targetReturn, trackingError, portfolioVol));
        }
    }

    void writeResults(const string& filename = "portfolio_analysis.csv") {
        ofstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Unable to open output file: " + filename);
        }

        file << fixed << setprecision(6);

        // 1. Portfolio Weights
        file << "Portfolio Weights Analysis\n";
        file << "Asset,TE Weight,MPT Weight\n";
        for (int i = 0; i < NUM_ASSETS; i++) {
            file << "Asset " << (i+1) << "," 
                 << teWeights_[i][0] << "," 
                 << mptWeights_[i][0] << "\n";
        }
        
        // 2. Performance Metrics
        file << "\nPerformance Metrics\n";
        file << "Metric,Value\n";
        file << "Daily Portfolio Return," << dailyReturn_ << "\n";
        file << "Monthly Portfolio Return," << monthlyReturn_ << "\n";
        file << "Daily SPY Return," << benchmarkReturn_ << "\n";
        file << "Monthly SPY Return," << pow((1 + benchmarkReturn_), TRADING_DAYS_PER_MONTH) - 1 << "\n";
        file << "Daily Portfolio Volatility," << dailyVol_ << "\n";
        file << "Monthly Portfolio Volatility," << monthlyVol_ << "\n";
        file << "Tracking Error," << trackingError_ << "\n";
        
        // 3. Efficient Frontier Data
        file << "\nEfficient Frontier\n";
        file << "Expected Return,Tracking Error,Portfolio Volatility\n";
        for (const auto& point : efficientFrontierPoints_) {
            file << get<0>(point) << "," 
                 << get<1>(point) << "," 
                 << get<2>(point) << "\n";
        }
        
        file.close();
    }

    // Getters for stress testing and risk reporting
    Matrix getReturns() const { return returns_; }
    Matrix getOptimalWeights() const { return teWeights_; }
    Matrix getCovariance() const { return covariance_; }
    Real getTrackingError() const { return trackingError_; }
};

int main() {
    try {
        cout << "Starting portfolio optimization..." << endl;

        // 1. Initialize and run core optimization
        EnhancedPortfolioOptimizer optimizer("12-stock portfolio.csv");
        optimizer.optimizePortfolio();
        optimizer.writeResults("portfolio_analysis.csv");
        
        // 2. Add stress testing
        cout << "Performing stress tests..." << endl;
        StressTesting::Scenario scenario;
        scenario.name = "Market Stress";
        scenario.marketShocks = vector<double>(12, -0.10);  // 10% market decline
        
        StressTesting stressTester(optimizer.getReturns());
        auto stressResults = stressTester.runStressTest(
            optimizer.getOptimalWeights(), scenario);
        
        // 3. Generate risk report
        cout << "Generating risk report..." << endl;
        RiskReporter::generateDetailedReport(
            "risk_report.txt",
            optimizer.getOptimalWeights(),
            optimizer.getReturns(),
            optimizer.getCovariance()
        );
        
        cout << "Optimization completed successfully." << endl;
        cout << "Results written to portfolio_analysis.csv" << endl;
        cout << "Risk report written to risk_report.txt" << endl;
        
        return 0;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}