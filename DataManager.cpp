#include "DataManager.hpp"
#include "Parser.hpp"
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <iostream>

void DataManager::loadData(const std::string& filename, 
                         const std::string& dateFormat,
                         bool adjustForDividends) {
    try {
        Parser portfolio(filename);
        dates_.clear();
        marketData_.clear();
        
        // Process data row by row
        for (size_t i = 1; i < portfolio.rowCount(); ++i) {
            boost::gregorian::date date = 
                boost::gregorian::from_string(portfolio[i][0]);
            dates_.push_back(date);
            
            for (size_t j = 0; j < portfolio.columnCount() - 1; ++j) {
                MarketData data;
                data.date = date;
                data.price = std::stod(portfolio[i][j + 1]);
                if (adjustForDividends) {
                    data.adjustedClose = std::stod(portfolio[i][j + 2]);
                } else {
                    data.adjustedClose = data.price;
                }
                data.volume = std::stod(portfolio[i][j + 3]);
                
                marketData_[portfolio[0][j + 1]].push_back(data);
            }
        }
        
        calculateReturns();
        validateData();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load data: " + std::string(e.what()));
    }
}

void DataManager::calculateReturns() {
    size_t numAssets = marketData_.size();
    size_t numDays = dates_.size();
    
    returns_ = Matrix(numDays - 1, numAssets);
    excessReturns_ = Matrix(numDays - 1, numAssets);
    
    size_t col = 0;
    for (const auto& [symbol, data] : marketData_) {
        for (size_t i = 1; i < data.size(); ++i) {
            double ret = (data[i].adjustedClose / data[i-1].adjustedClose) - 1.0;
            returns_[i-1][col] = ret;
            excessReturns_[i-1][col] = ret - benchmarkReturns_[i-1][0];
        }
        ++col;
    }
}

void DataManager::validateData() {
    validateDateContinuity();
    detectOutliers();
    checkMissingValues();
}

void DataManager::validateDateContinuity() {
    for (size_t i = 1; i < dates_.size(); ++i) {
        auto diff = dates_[i] - dates_[i-1];
        if (diff.days() > 5) {
            throw std::runtime_error("Data gap detected between " + 
                boost::gregorian::to_simple_string(dates_[i-1]) + " and " + 
                boost::gregorian::to_simple_string(dates_[i]));
        }
    }
}

void DataManager::detectOutliers() {
    for (auto& [symbol, data] : marketData_) {
        std::vector<double> prices;
        prices.reserve(data.size());
        for (const auto& point : data) {
            prices.push_back(point.price);
        }
        
        double mean = std::accumulate(prices.begin(), prices.end(), 0.0) / prices.size();
        double sqSum = std::inner_product(prices.begin(), prices.end(), 
                                        prices.begin(), 0.0);
        double stdDev = std::sqrt(sqSum / prices.size() - mean * mean);
        
        for (size_t i = 0; i < prices.size(); ++i) {
            if (std::abs(prices[i] - mean) > 5 * stdDev) {
                std::cerr << "Warning: Outlier detected in " << symbol 
                         << " at date " << boost::gregorian::to_simple_string(data[i].date) 
                         << std::endl;
            }
        }
    }
}

void DataManager::checkMissingValues() {
    for (const auto& [symbol, data] : marketData_) {
        for (const auto& point : data) {
            if (std::isnan(point.price) || std::isinf(point.price)) {
                throw std::runtime_error("Invalid price data for " + symbol + 
                    " at date " + boost::gregorian::to_simple_string(point.date));
            }
        }
    }
}

double DataManager::calculateBeta(const Matrix& assetReturns, 
                                const Matrix& benchmarkReturns) {
    double covariance = 0.0;
    double benchmarkVariance = 0.0;
    double meanAsset = 0.0;
    double meanBenchmark = 0.0;
    
    // Calculate means
    for (Size i = 0; i < assetReturns.rows(); ++i) {
        meanAsset += assetReturns[i][0];
        meanBenchmark += benchmarkReturns[i][0];
    }
    meanAsset /= assetReturns.rows();
    meanBenchmark /= benchmarkReturns.rows();
    
    // Calculate covariance and variance
    for (Size i = 0; i < assetReturns.rows(); ++i) {
        covariance += (assetReturns[i][0] - meanAsset) * 
                     (benchmarkReturns[i][0] - meanBenchmark);
        benchmarkVariance += std::pow(benchmarkReturns[i][0] - meanBenchmark, 2);
    }
    
    covariance /= (assetReturns.rows() - 1);
    benchmarkVariance /= (benchmarkReturns.rows() - 1);
    
    return covariance / benchmarkVariance;
}

double DataManager::calculateVolatility(const Matrix& returns) {
    double mean = 0.0;
    double variance = 0.0;
    
    // Calculate mean
    for (Size i = 0; i < returns.rows(); ++i) {
        mean += returns[i][0];
    }
    mean /= returns.rows();
    
    // Calculate variance
    for (Size i = 0; i < returns.rows(); ++i) {
        variance += std::pow(returns[i][0] - mean, 2);
    }
    variance /= (returns.rows() - 1);
    
    // Annualize volatility (sqrt(252) for daily data)
    return std::sqrt(variance * 252);
}

Matrix DataManager::calculateRollingBeta(int windowSize) {
    size_t numAssets = marketData_.size();
    size_t numDays = dates_.size();
    Matrix betas(numDays - windowSize, numAssets);
    
    for (size_t i = windowSize; i < numDays; ++i) {
        Matrix windowReturns = returns_.range(i - windowSize, i, 0, numAssets);
        Matrix windowBenchmark = benchmarkReturns_.range(i - windowSize, i, 0, 1);
        
        for (size_t j = 0; j < numAssets; ++j) {
            betas[i-windowSize][j] = calculateBeta(windowReturns.column(j), 
                                                 windowBenchmark.column(0));
        }
    }
    
    return betas;
}

Matrix DataManager::calculateRollingVolatility(int windowSize) {
    size_t numAssets = marketData_.size();
    size_t numDays = dates_.size();
    Matrix vols(numDays - windowSize, numAssets);
    
    for (size_t i = windowSize; i < numDays; ++i) {
        Matrix windowReturns = returns_.range(i - windowSize, i, 0, numAssets);
        
        for (size_t j = 0; j < numAssets; ++j) {
            vols[i-windowSize][j] = calculateVolatility(windowReturns.column(j));
        }
    }
    
    return vols;
}

std::vector<double> DataManager::calculateDrawdowns() {
    std::vector<double> drawdowns;
    drawdowns.reserve(dates_.size());
    
    double peak = 1.0;
    double value = 1.0;
    
    for (size_t i = 0; i < returns_.rows(); ++i) {
        value *= (1 + returns_[i][0]);
        peak = std::max(peak, value);
        drawdowns.push_back((peak - value) / peak);
    }
    
    return drawdowns;
}

const Matrix& DataManager::getCorrelationMatrix() {
    if (!correlationMatrix_) {
        // Calculate if not cached
        correlationMatrix_ = std::make_unique<Matrix>(returns_.columns(), 
                                                    returns_.columns());
        // Calculate correlation matrix
        // ... implementation details
    }
    return *correlationMatrix_;
}

const Matrix& DataManager::getCovarianceMatrix() {
    if (!covarianceMatrix_) {
        // Calculate if not cached
        covarianceMatrix_ = std::make_unique<Matrix>(returns_.columns(), 
                                                   returns_.columns());
        // Calculate covariance matrix
        // ... implementation details
    }
    return *covarianceMatrix_;
}