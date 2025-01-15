#pragma once
#include <ql/quantlib.hpp>
#include <memory>
#include <unordered_map>
#include <boost/date_time.hpp>
#include <vector>
#include <string>

class Parser; // Forward declaration

class DataManager {
private:
    struct MarketData {
        boost::gregorian::date date;
        double price;
        double volume;
        double adjustedClose;
    };

    std::unordered_map<std::string, std::vector<MarketData>> marketData_;
    Matrix returns_;
    Matrix excessReturns_;
    Matrix benchmarkReturns_;
    std::vector<boost::gregorian::date> dates_;
    
    // Cache for performance
    std::unique_ptr<Matrix> correlationMatrix_;
    std::unique_ptr<Matrix> covarianceMatrix_;

    // Private helper methods
    void calculateReturns();
    void validateData();
    void validateDateContinuity();
    void detectOutliers();
    void checkMissingValues();
    double calculateBeta(const Matrix& assetReturns, const Matrix& benchmarkReturns);
    double calculateVolatility(const Matrix& returns);

public:
    // Constructor
    DataManager() = default;
    
    // Main data loading method
    void loadData(const std::string& filename, 
                 const std::string& dateFormat = "%Y-%m-%d",
                 bool adjustForDividends = true);

    // Analysis methods
    Matrix calculateRollingBeta(int windowSize = 60);
    Matrix calculateRollingVolatility(int windowSize = 20);
    std::vector<double> calculateDrawdowns();

    // Getters
    const Matrix& getReturns() const { return returns_; }
    const Matrix& getExcessReturns() const { return excessReturns_; }
    const Matrix& getBenchmarkReturns() const { return benchmarkReturns_; }
    const std::vector<boost::gregorian::date>& getDates() const { return dates_; }
    const Matrix& getCorrelationMatrix();
    const Matrix& getCovarianceMatrix();
};