#pragma once

// Add common includes used across multiple files
#include <ql/quantlib.hpp>
#include <boost/date_time.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cmath>

// Add common type definitions
using Matrix = QuantLib::Matrix;
using Size = QuantLib::Size;
using Date = boost::gregorian::date; 