# Portfolio Optimization with Tracking Error and Monthly Rebalancing

## Overview
This project implements a sophisticated portfolio optimization strategy that combines tracking error minimization with monthly rebalancing and transaction cost considerations. The implementation uses C++ with QuantLib integration for robust financial calculations.

## Features
- **Tracking Error Optimization**: Minimizes portfolio deviation from benchmark (SPY)
- **Monthly Rebalancing**: Systematic portfolio rebalancing with transaction cost considerations
- **Transaction Cost Model**: Includes:
  - Fixed commission costs
  - Variable commission rates
  - Market impact modeling
  - Slippage estimation
- **Risk Analysis**: Comprehensive risk metrics and stress testing
- **Performance Reporting**: Detailed performance analytics and reporting

## Project Structure
```
Tracking-Error-and-Portfolio-Optimization-master/
├── src/
│   ├── PortfolioOptimizer.hpp
│   ├── PortfolioOptimizer.cpp
│   ├── PortfolioRebalancer.hpp
│   ├── PortfolioRebalancer.cpp
│   ├── TransactionCostModel.hpp
│   ├── TransactionCostModel.cpp
│   ├── StressTesting.hpp
│   ├── RiskReporter.hpp
│   └── weight.cpp
├── data/
│   └── 12-stock portfolio.csv
└── README.md
```

## Dependencies
- **QuantLib**: Financial mathematics and modeling
- **Eigen**: Linear algebra operations
- **Boost**: Additional C++ utilities
- **C++11 or higher**

## Installation
1. Install required dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install libquantlib0-dev libeigen3-dev libboost-all-dev

# macOS
brew install quantlib eigen boost
```

2. Clone the repository:
```bash
git clone https://github.com/yourusername/Tracking-Error-and-Portfolio-Optimization.git
cd Tracking-Error-and-Portfolio-Optimization
```

3. Build the project:
```bash
mkdir build && cd build
cmake ..
make
```

## Usage
1. Prepare your input data in CSV format (see Data Format section)
2. Run the optimization:
```bash
./portfolio_optimizer path/to/data.csv
```

## Data Format
The project expects historical price data in CSV format with the following structure:

```csv
Dates,MSFT,F,BGS,ADRD,V,MGI,NFLX,JACK,GE,SBUX,C,HD,SPY
6/29/2018,-0.000202,-0.018617,-0.055292,0.008062,-0.002184,0.009049,-0.010090,-0.016522,-0.015907,0.006386,0.000598,-0.000563,0.005717
...
```

- First column: Dates
- Middle columns: Daily returns for 12 stocks
- Last column: SPY (benchmark) returns

## Key Components

### 1. Portfolio Optimizer
- Implements tracking error minimization
- Calculates efficient frontier
- Optimizes portfolio weights

### 2. Transaction Cost Model
- Models trading costs including:
  - Fixed commissions
  - Variable commissions
  - Market impact
  - Slippage

### 3. Portfolio Rebalancer
- Monthly rebalancing logic
- Transaction cost consideration
- Trade execution modeling

### 4. Risk Analysis
- Tracking error calculation
- Volatility metrics
- Stress testing scenarios

## Output Files
The program generates several output files:
- `portfolio_YYYY-MM-DD.csv`: Portfolio weights and metrics
- `risk_report_YYYY-MM-DD.txt`: Risk analysis reports
- `final_portfolio_analysis.csv`: Final optimization results

## Performance Metrics
- Daily/Monthly Returns
- Portfolio Volatility
- Tracking Error
- Transaction Costs
- Information Ratio

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

## Acknowledgments
- QuantLib Documentation
- Modern Portfolio Theory
- Transaction Cost Analysis Research
