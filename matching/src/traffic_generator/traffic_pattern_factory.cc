// traffic_pattern_factory.cc file

#include "traffic_pattern_factory.h"

namespace saber {
// Implementation of Create API for class TrafficPatternFactory
std::unique_ptr<TrafficPattern> TrafficPatternFactory::Create(TrafficPatternName pattern_name, 
        int node_num, std::mt19937::result_type seed) {

  TrafficPattern *result = nullptr;
  if (pattern_name == TrafficPatternName::Uniform) {
    result = new UniformTrafficPattern(node_num, seed);
  } else if (pattern_name == TrafficPatternName::LogDiagonal) {
    result = new LogDiagonalTrafficPattern(node_num, seed);
  } else if (pattern_name == TrafficPatternName::QuasiDiagonal) {
    result = new QuasiDiagonalTrafficPattern(node_num, seed);
  } else if (pattern_name == TrafficPatternName::Diagonal) {
    result = new DiagonalTrafficPattern(node_num, seed);
  } else {
    std::cerr << "Unknown pattern name!\n";
  }

  return std::unique_ptr<TrafficPattern> (result);
}

std::vector<std::vector<int>> static_traffic_matrix(
      int row_sum, int row_num, int col_num, TrafficPattern& pattern){
  std::vector<std::vector<int>> result;
  result.reserve(row_num);

  for(int row = 0; row < row_num; ++row){
    std::vector<int> vec(col_num, 0);
    for(int packet = 0; packet < row_sum; ++packet){
      int dest = pattern.destination(row);
      ++vec[dest];
    }
    result.push_back(std::move(vec));
  }
  return result;
}
} // namespace saber
