# Portfolio Optimization Project

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Implementation](#implementation)
- [Usage](#usage)
- [Output](#output)
- [Future Work](#future-work)

## Overview

### Project Description
A quantitative portfolio optimization system implementing both Markowitz Modern Portfolio Theory (MPT) and Tracking Error optimization strategies. The system analyzes 12 stocks against the SPY benchmark using historical data.

### Key Statistics
Time Period: 4/1/2015 - 6/1/2018
Trading Days: 847
Assets: 12 stocks + SPY benchmark
Data Frequency: Daily returns

## Features

### Core Functionality
- [x] Markowitz Portfolio Optimization
- [x] Tracking Error Minimization
- [x] Efficient Frontier Generation
- [x] Risk Metrics Calculation
- [x] Stress Testing
- [x] Performance Analysis

### Data Processing
graph LR
    A[CSV Input] --> B[Data Loading]
    B --> C[Returns Processing]
    C --> D[Covariance Calculation]
    D --> E[Optimization]
    E --> F[Output Generation]

## Architecture

### Class Structure
```cpp
class EnhancedPortfolioOptimizer {
    // Core data structures
    Matrix returns_;
    Matrix excessReturns_;
    Matrix covariance_;
    Matrix excessCovariance_;
    
    // Performance metrics
    Real dailyReturn_;
    Real monthlyReturn_;
    Real dailyVol_;
    Real monthlyVol_;
    Real trackingError_;
}
```

### Key Components

#### 1. Portfolio Optimizer
```cpp
// Core optimization method
Matrix calculateMarkowitzWeights(
    const Matrix& mu,      // Expected returns
    const Matrix& sigma,   // Covariance matrix
    const Matrix& u,       // Unit vector
    Real targetReturn,     // Target return
    Real& optMu,          // Optimal return
    Real& optSigmaSq      // Optimal variance
);
```

#### 2. Risk Reporter
```cpp
// Risk reporting functionality
void generateDetailedReport(
    const string& filename,
    const Matrix& weights,
    const Matrix& returns,
    const Matrix& covariance
);
```

#### 3. Stress Testing
```cpp
struct Scenario {
    string name;
    vector<double> marketShocks;
    vector<double> volatilityShocks;
    vector<double> correlationShocks;
};
```

## Implementation

### Data Input Format
```csv
Index,Date,Stock1,Stock2,...,Stock12,SPY
0,6/1/2018,0.001,0.002,...,0.003,0.002
1,5/31/2018,0.002,0.001,...,0.001,0.001
...
```

### Core Components

#### 1. Data Loading and Processing
```cpp
void loadData(const string& filename) {
    Parser portfolio(filename);
    returns_ = Matrix(NUM_PERIODS, NUM_ASSETS);
    excessReturns_ = Matrix(NUM_PERIODS, NUM_ASSETS);
    
    for (int i = 0; i < NUM_PERIODS; i++) {
        for (int j = 0; j < NUM_ASSETS; j++) {
            returns_[i][j] = stod(portfolio[i][j + FIRST_ASSET_COLUMN]);
            excessReturns_[i][j] = returns_[i][j] - 
                                  stod(portfolio[i][BENCHMARK_COLUMN]);
        }
    }
}
```

#### 2. Optimization Process
```cpp
void optimizePortfolio() {
    // Initialize parameters
    Real targetReturn = 0.0013;
    Matrix u(NUM_ASSETS, 1, 1.0);
    Real optMu = 0.0, optSigmaSq = 0.001;
    
    // Tracking Error Optimization
    teWeights_ = calculateMarkowitzWeights(
        excessReturns_, excessCovariance_, u, targetReturn, 
        optMu, optSigmaSq);
    
    // Traditional MPT Optimization
    mptWeights_ = calculateMarkowitzWeights(
        returns_, covariance_, u, targetReturn + benchmarkReturn_, 
        optMu, optSigmaSq);
}
```

#### 3. Performance Metrics
```cpp
void calculatePerformanceMetrics() {
    dailyReturn_ = (transpose(teWeights_)*returns_)[0][0];
    dailyVol_ = sqrt((transpose(teWeights_)*covariance_*teWeights_)[0][0]);
    trackingError_ = sqrt((transpose(teWeights_)*excessCovariance_*teWeights_)[0][0]);
    monthlyReturn_ = pow(1 + dailyReturn_, TRADING_DAYS_PER_MONTH) - 1;
    monthlyVol_ = dailyVol_ * sqrt(TRADING_DAYS_PER_MONTH);
}
```

## Usage

### Installation
```bash
git clone <repository-url>
cd Tracking-Error-and-Portfolio-Optimization
```

### Running the Program
```cpp
int main() {
    try {
        cout << "Starting portfolio optimization..." << endl;

        // Initialize and run optimization
        EnhancedPortfolioOptimizer optimizer("12-stock portfolio.csv");
        optimizer.optimizePortfolio();
        optimizer.writeResults("portfolio_analysis.csv");
        
        // Perform stress testing
        StressTesting::Scenario scenario;
        scenario.name = "Market Stress";
        scenario.marketShocks = vector<double>(12, -0.10);
        
        StressTesting stressTester(optimizer.getReturns());
        auto stressResults = stressTester.runStressTest(
            optimizer.getOptimalWeights(), scenario);
        
        // Generate risk report
        RiskReporter::generateDetailedReport(
            "risk_report.txt",
            optimizer.getOptimalWeights(),
            optimizer.getReturns(),
            optimizer.getCovariance()
        );
        
        return 0;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
```

## Output

### Portfolio Analysis (portfolio_analysis.csv)
```csv
Portfolio Weights Analysis
Asset,TE Weight,MPT Weight
Asset 1,0.123,0.145
Asset 2,0.089,0.092
...

Performance Metrics
Metric,Value
Daily Portfolio Return,0.00134
Monthly Portfolio Return,0.0295
Daily Portfolio Volatility,0.0156
Monthly Portfolio Volatility,0.0732
Tracking Error,0.0089
```

### Risk Report (risk_report.txt)
```
Risk Analysis Report
------------------
Portfolio Characteristics:
- Daily Return: 0.00134
- Monthly Return: 0.0295
- Daily Volatility: 0.0156
- Monthly Volatility: 0.0732
- Tracking Error: 0.0089
- Information Ratio: 0.456
- Sharpe Ratio: 0.892

Stress Test Results:
- Maximum Drawdown: 0.0845
- VaR (95%): 0.0256
- Expected Shortfall: 0.0312
```

## Future Work

### Planned Enhancements
- [ ] Multi-factor risk model integration
- [ ] Real-time market data feed
- [ ] Machine learning-based return predictions
- [ ] Advanced transaction cost modeling
- [ ] Dynamic rebalancing optimization

### Performance Improvements
- [ ] Parallel computation for covariance matrix
- [ ] GPU acceleration for optimization
- [ ] Improved memory management for large datasets

## Dependencies
- QuantLib (>= 1.15)
- Eigen (>= 3.3)
- Boost (>= 1.65)
- C++17 or later

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing
1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Authors
* **Yuting Hou** - *Initial work*

## Acknowledgments
* QuantLib community for the financial library
* Modern Portfolio Theory by Harry Markowitz
* Tracking Error Optimization methodology
