# Portfolio Optimization with Tracking Error

## Overview
This project implements a sophisticated portfolio optimization system that focuses on minimizing tracking error while maintaining desired risk-return characteristics. Built in C++, it provides a comprehensive framework for institutional-grade portfolio management with risk constraints and transaction cost considerations.

## Table of Contents
- [Architecture](#architecture)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [API Documentation](#api-documentation)
- [Performance Metrics](#performance-metrics)
- [Examples](#examples)

## Architecture

### Project Structure
```
Tracking-Error-and-Portfolio-Optimization-master/
├── src/
│   ├── PortfolioOptimizer.hpp/cpp     # Core optimization engine
│   ├── RiskMetrics.hpp/cpp            # Risk calculations
│   ├── RiskConstraints.hpp/cpp        # Portfolio constraints
│   ├── TransactionCostModel.hpp/cpp   # Trading cost analysis
│   ├── CSVParser.hpp                  # Data handling
│   └── weight.cpp                     # Main execution
├── data/
│   └── portfolio_data.csv             # Historical price data
├── tests/
│   └── unit_tests.cpp                 # Test suite
└── docs/
    └── api_documentation.md           # API documentation
```

### Core Components

1. **Portfolio Optimizer**
   - Implements Markowitz optimization
   - Tracking error minimization
   - Portfolio rebalancing logic
   - Integration with risk constraints

2. **Risk Metrics System**
   ```cpp
   struct PortfolioRisk {
       double dailyVol;
       double monthlyVol;
       double trackingError;
       double informationRatio;
       double sharpeRatio;
       double maxDrawdown;
       double beta;
       double alpha;
   };
   ```

3. **Risk Constraints Framework**
   ```cpp
   struct ConstraintLimits {
       double maxPositionSize;      // 20% max position
       double minPositionSize;      // -10% min position
       double maxSectorExposure;    // 30% sector limit
       double maxVolatility;        // 15% volatility cap
       double maxTrackingError;     // 5% tracking error limit
       double maxTurnover;          // 20% monthly turnover
   };
   ```

## Features

### 1. Portfolio Optimization
- Tracking error minimization
- Efficient frontier calculation
- Risk-adjusted return optimization
- Transaction cost consideration

### 2. Risk Management
- Comprehensive risk metrics calculation
- Position and sector limits
- Volatility targeting
- Beta neutrality options

### 3. Transaction Cost Analysis
- Fixed commission modeling
- Variable commission rates
- Market impact estimation
- Liquidity constraints

## Installation

### Prerequisites
- C++17 or higher
- CMake 3.10+
- QuantLib
- Eigen3
- Boost

### Build Instructions
```bash
git clone https://github.com/yourusername/Tracking-Error-and-Portfolio-Optimization.git
cd Tracking-Error-and-Portfolio-Optimization
mkdir build && cd build
cmake ..
make
```

## Usage

### Basic Example
```cpp
#include "PortfolioOptimizer.hpp"

int main() {
    // Initialize optimizer
    EnhancedPortfolioOptimizer optimizer("portfolio_data.csv");
    
    // Set risk constraints
    RiskConstraints::ConstraintLimits limits;
    limits.maxPositionSize = 0.15;    // 15% max position
    limits.maxSectorExposure = 0.25;  // 25% sector limit
    
    // Optimize portfolio
    optimizer.optimizePortfolio();
    
    // Generate risk report
    optimizer.generateRiskReport("risk_report.txt");
    
    return 0;
}
```

### Data Format
The system expects CSV data in the following format:
```csv
Date,Asset1,Asset2,Asset3,...,AssetN,Benchmark
2023-01-01,0.0012,0.0034,-0.0015,...,0.0023,0.0018
```

## API Documentation

### EnhancedPortfolioOptimizer
```cpp
class EnhancedPortfolioOptimizer {
public:
    // Constructor
    EnhancedPortfolioOptimizer(const string& filename, int windowSize = 252);
    
    // Core methods
    void optimizePortfolio();
    void generateRiskReport(const string& filename);
    
    // Getters
    Matrix getOptimizedWeights() const;
    RiskMetrics::PortfolioRisk getCurrentRisk() const;
};
```

### RiskMetrics
```cpp
class RiskMetrics {
public:
    // Risk calculation methods
    PortfolioRisk calculateRiskMetrics(const Matrix& weights, ...);
    double calculateTrackingError(const Matrix& weights, ...);
    double calculateVolatility(const Matrix& weights, ...);
};
```

## Performance Metrics

### Risk Measures
- Daily/Monthly Volatility
- Tracking Error
- Information Ratio
- Sharpe Ratio
- Maximum Drawdown
- Beta/Alpha

### Transaction Costs
- Commission Rates
- Market Impact
- Slippage
- Total Trading Costs

## Examples

### 1. Basic Portfolio Optimization
```cpp
// Initialize optimizer
EnhancedPortfolioOptimizer optimizer("data.csv");

// Optimize portfolio
optimizer.optimizePortfolio();

// Get optimized weights
Matrix weights = optimizer.getOptimizedWeights();
```

### 2. Risk-Constrained Optimization
```cpp
// Set risk constraints
RiskConstraints::ConstraintLimits limits;
limits.maxPositionSize = 0.15;
limits.maxVolatility = 0.20;
limits.maxTrackingError = 0.06;

// Apply constraints and optimize
optimizer.setRiskConstraints(limits);
optimizer.optimizePortfolio();
```

### 3. Transaction Cost Analysis
```cpp
// Set transaction cost parameters
TransactionCostModel::Costs costs;
costs.fixedCommission = 0.0001;    // 1 bp
costs.marketImpact = 0.1;          // Impact coefficient

// Optimize with transaction costs
optimizer.setCostModel(costs);
optimizer.optimizePortfolio();
```

## Contributing
1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Contact
Your Name - your.email@example.com
Project Link: https://github.com/yourusername/Tracking-Error-and-Portfolio-Optimization
