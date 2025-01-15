# Portfolio Optimization with Tracking Error and Monthly Rebalancing

## Project Overview
This project implements a sophisticated portfolio optimization strategy that combines tracking error minimization with systematic monthly rebalancing. Built in C++ with QuantLib integration, it provides a robust framework for institutional-grade portfolio management.

## Architecture

### Core Components
```
Tracking-Error-and-Portfolio-Optimization-master/
├── src/
│   ├── PortfolioOptimizer.hpp/cpp     # Core optimization engine
│   ├── PortfolioRebalancer.hpp/cpp    # Rebalancing logic
│   ├── TransactionCostModel.hpp/cpp   # Transaction cost analysis
│   ├── StressTesting.hpp             # Risk scenario analysis
│   ├── RiskReporter.hpp              # Performance reporting
│   └── weight.cpp                    # Main execution file
├── data/
│   └── 12-stock portfolio.csv        # Historical price data
└── README.md
```

### Component Interactions
1. **PortfolioOptimizer**
   - Implements tracking error minimization
   - Calculates efficient frontier
   - Manages rolling window calculations
   - Interfaces with QuantLib for matrix operations

2. **PortfolioRebalancer**
   - Handles monthly rebalancing decisions
   - Integrates with TransactionCostModel
   - Maintains position tracking
   - Implements rebalancing thresholds

3. **TransactionCostModel**
   - Models trading costs with multiple components:
     ```cpp
     struct Costs {
         double fixedCommission;      // Fixed fee per trade
         double variableCommission;   // Percentage-based fee
         double slippageModel;        // Price impact
         double marketImpact;         // Market depth impact
     };
     ```

## Data Structure

### Input Data Format
The system uses a CSV file with the following structure:
```csv
Dates,MSFT,F,BGS,ADRD,V,MGI,NFLX,JACK,GE,SBUX,C,HD,SPY
6/29/2018,-0.000202,-0.018617,...,0.005717
```
- First column: Trading dates
- Columns 2-13: Daily returns for 12 stocks
- Last column: SPY (benchmark) returns

### Key Data Processing
1. **Return Calculations**
   - Daily returns processing
   - Rolling window statistics
   - Excess return computation

2. **Covariance Estimation**
   - Rolling covariance matrix
   - Exponential weighting
   - Regularization techniques

## Optimization Process

### 1. Initial Portfolio Construction
```cpp
class PortfolioOptimizer {
    Matrix calculateMarkowitzWeights(
        const Matrix& mu,          // Expected returns
        const Matrix& sigma,       // Covariance matrix
        const Matrix& u,          // Unit vector
        Real targetReturn,        // Target portfolio return
        Real& optMu,             // Optimal return
        Real& optSigmaSq        // Optimal variance
    );
};
```

### 2. Monthly Rebalancing
- Systematic review on month-end dates
- Transaction cost consideration
- Position size limits
- Minimum trade size filters

### 3. Risk Management
- Tracking error monitoring
- Volatility constraints
- Stress testing scenarios
- Position limits enforcement

## Performance Analytics

### Key Metrics
1. **Return Measures**
   - Daily/Monthly returns
   - Rolling window performance
   - Risk-adjusted returns

2. **Risk Metrics**
   - Tracking error
   - Portfolio volatility
   - Information ratio
   - Beta to benchmark

3. **Cost Analysis**
   - Transaction costs
   - Market impact
   - Portfolio turnover
   - Implementation shortfall

## Dependencies
- **QuantLib**: Financial mathematics and modeling
- **Eigen**: Linear algebra operations
- **Boost**: Additional C++ utilities
- **C++11 or higher**

## Installation

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install libquantlib0-dev libeigen3-dev libboost-all-dev

# macOS
brew install quantlib eigen boost
```

### Build Process
```bash
git clone https://github.com/yourusername/Tracking-Error-and-Portfolio-Optimization.git
cd Tracking-Error-and-Portfolio-Optimization
mkdir build && cd build
cmake ..
make
```

## Usage

### Basic Execution
```bash
./portfolio_optimizer path/to/data.csv
```

### Configuration Options
- Rolling window size (default: 252 days)
- Rebalancing frequency (default: monthly)
- Transaction cost parameters
- Risk limits and constraints

## Output Files

### 1. Portfolio Analysis
- `portfolio_YYYY-MM-DD.csv`: Monthly portfolio weights
- `risk_report_YYYY-MM-DD.txt`: Risk analytics
- `final_portfolio_analysis.csv`: Cumulative results

### 2. Performance Reports
- Portfolio weights
- Risk metrics
- Transaction costs
- Tracking error analysis

## Contributing
1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Contact
Your Name - your.email@example.com
Project Link: https://github.com/yourusername/Tracking-Error-and-Portfolio-Optimization

## Acknowledgments
- QuantLib Documentation
- Modern Portfolio Theory research
- Transaction Cost Analysis literature
