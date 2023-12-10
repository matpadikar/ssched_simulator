// traffic_pattern_factory.h header file

#ifndef TRAFFIC_PATTERN_FACTORY_H
#define TRAFFIC_PATTERN_FACTORY_H

#include <iostream>

#include "diagonal_traffic_pattern.h"
#include "log_diagonal_traffic_pattern.h"
#include "quasi_diagonal_traffic_pattern.h"
#include "traffic_pattern.h"
#include "uniform_traffic_pattern.h"
#include <memory>
#include <vector>

namespace saber {
enum class TrafficPatternName{
  Uniform = 0, QuasiDiagonal = 1, LogDiagonal = 2, Diagonal = 3
};
class TrafficPatternFactory {
public:
  static std::unique_ptr<TrafficPattern> Create(TrafficPatternName name, int node_num, std::mt19937::result_type seed);
};

std::vector<std::vector<int>> static_traffic_matrix(
      int row_sum, int row_num, int col_num, TrafficPattern& pattern);


} // namespace saber
#endif // TRAFFIC_PATTERN_FACTORY_H
