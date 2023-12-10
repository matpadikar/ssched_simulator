
#include "sb_qps.h"
#include <switch/iq_switch.h>
#include <queue>
#include <functional>

namespace saber {

/**
 * Start of implementation for class SB_QPS_Basic
 */

SB_QPS_Basic::SB_QPS_Basic(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0))
  { 
    _cf_rel_time = 0;
    bstLeftStart = BST::nearest_power_of_two(_num_outputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
    }
   /// generate initial schedulers (used for the first batch)
    for (auto &schedul: schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_Basic::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_Basic::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_Basic::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_Basic::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if(outAccepts[out].empty()){
      outAccepts[out].push_back(in);
    } else if (VOQLength[in][out] > VOQLength[outAccepts[out].front()][out]){
      outAccepts[out][0] = in;
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()) continue;
    int in = outAccepts[out][0];
    inputMatchBitmap[in].set(timeSlot);
    ouputMatchBitmap[out].set(timeSlot);
    schedules[timeSlot][in] = out;
    virtualDepartures.emplace_back(in, out);
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_Basic::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_Basic::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto input = srcDestPair.first;
    auto output = srcDestPair.second;
    assert(VOQLength[input][output] > 0);

    --VOQLength[input][output];
    BST::update<int>(bstForSamplingAtInput[input], output + bstLeftStart, -1);
  }
}

void SB_QPS_Basic::reset(){
  BatchScheduler::reset();
  resetBitmap();
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
    std::fill(VOQLength[i].begin(), VOQLength[i].end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_Basic::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
}

void SB_QPS_Basic::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_Basic



/**
 * Start of implementation for class QB_QPS_HalfHalf
 */

SB_QPS_HalfHalf_Oblivious::SB_QPS_HalfHalf_Oblivious(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)) 
  {
    bstLeftStart = BST::nearest_power_of_two(_num_outputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
    }
    _cf_rel_time = 0;
   /// generate initial schedules (used for the first batch)
    for (auto &schedul: schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_HalfHalf_Oblivious::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_HalfHalf_Oblivious::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    ++VOQLength[srcDestPair.first][srcDestPair.second];
    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart, 1);
  }
}

void SB_QPS_HalfHalf_Oblivious::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_HalfHalf_Oblivious::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  /// maximum number of accepts for each time slots
  int maxAccepts = (((timeSlot + 1) * 2 > getBatchSize()) ? 2 : 1);

  std::vector<std::array<int, 2>> outAccepts(num_outputs());
  for (auto &oac: outAccepts){
    oac.fill(-1);
  }

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (outAccepts[out][0] == -1){
      outAccepts[out][0] = in;
    } else if (VOQLength[in][out] > VOQLength[outAccepts[out][0]][out]){
      outAccepts[out][1] = outAccepts[out][0];
      outAccepts[out][0] = in;
    } else if (outAccepts[out][1] == -1 || (VOQLength[in][out] > VOQLength[outAccepts[out][1]][out])){
      outAccepts[out][1] = in;
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out][0] == -1){
      continue;
    }

    /// Schedule input with largest VOQ for current time slot
    int in = outAccepts[out][0];
    inputMatchBitmap[in].set(timeSlot);
    ouputMatchBitmap[out].set(timeSlot);
    schedules[timeSlot][in] = out;
    virtualDepartures.emplace_back(in, out);

    /// Schedule input with 2nd largest VOQ for earlier available time slot if any
    if (maxAccepts > 1 && outAccepts[out][1] != -1){ 
      int in = outAccepts[out][1];
      auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);/// to check both input and output available
      for (int f = (int) (timeSlot) - 1; f >= 0; --f){
        if (!isAlreadyMatched.test(f)){
          inputMatchBitmap[in].set(f);
          ouputMatchBitmap[out].set(f);
          assert(schedules[f][in] == -1);
          schedules[f][in] = out;

          virtualDepartures.emplace_back(in, out);
          break;/// pay special attention (first fit)
        }
      }
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_HalfHalf_Oblivious::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_HalfHalf_Oblivious::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto input = srcDestPair.first;
    auto output = srcDestPair.second;
    assert(VOQLength[input][output] > 0);

    --VOQLength[input][output];
    BST::update<int>(bstForSamplingAtInput[input], output + bstLeftStart, -1);
  }
}

void SB_QPS_HalfHalf_Oblivious::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
    std::fill(VOQLength[i].begin(), VOQLength[i].end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_Oblivious::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
}

void SB_QPS_HalfHalf_Oblivious::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_HalfHalf_Oblivious



/**
 * Start of implementation for class SB_QPS_HalfHalf_AvailabilityAware
 */
SB_QPS_HalfHalf_AvailabilityAware::SB_QPS_HalfHalf_AvailabilityAware(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)),
  outputAvailability(_num_outputs, 0),
  inputAvailability(_num_inputs, 0)
  {
  bstLeftStart = BST::nearest_power_of_two(_num_outputs);
  for (size_t i = 0; i < _num_inputs; ++i){
    bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
  }
  _cf_rel_time = 0;
  /// generate initial schedulers (used for the first batch)
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_AvailabilityAware::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_HalfHalf_AvailabilityAware::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_HalfHalf_AvailabilityAware::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_HalfHalf_AvailabilityAware::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  /// maximum number of accepts for each time slots
  int maxAccepts = (((timeSlot + 1) * 2 > getBatchSize()) ? (num_inputs()) : 1);

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (maxAccepts > 1 ){
      outAccepts[out].push_back(in);
    } else {/// normal QPS
      if(outAccepts[out].empty()){
        outAccepts[out].push_back(in);
      } else if (VOQLength[outAccepts[out].front()][out] < VOQLength[in][out]){
        outAccepts[out][0] = in;
      }
    }

  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures
  std::vector<int> out_match(num_outputs(), -1);

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()){
      continue;
    }

    /// Schedule input with the largest VOQ proposal for current time slot
    int in = outAccepts[out][0];
    inputMatchBitmap[in].set(timeSlot);
    ouputMatchBitmap[out].set(timeSlot);
    schedules[timeSlot][in] = out;
    out_match[out] = in;
    virtualDepartures.emplace_back(in, out);

    /// Schedule inputs with remaining VOQs proposals for earlier available time slots
    if (maxAccepts > 1){
      /// sort proposals based on VOQ lengths
      std::sort(outAccepts[out].begin(), outAccepts[out].end(), [=](const int in1, const int in2){
        return VOQLength[in1][out] > VOQLength[in2][out];
      });

      for (int i = 1; i < std::min(maxAccepts, (int)outAccepts[out].size()) && outputAvailability[out] > 0; ++i){
        int in = outAccepts[out][i];
        if (inputAvailability[in] == 0){
          continue;
        }

        /// available only when both available
        auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);
        for (int f = (int) (timeSlot) - 1; f >= 0; --f){
          if (!isAlreadyMatched.test(f)){
            inputMatchBitmap[in].set(f);
            ouputMatchBitmap[out].set(f);
            --inputAvailability[in];
            --outputAvailability[out];
            assert(schedules[f][in] == -1);
            schedules[f][in] = out;
            virtualDepartures.emplace_back(in, out);
            break;/// pay special attention (first fit)
          }
        }
      }
    }
  }

  for (int in = 0;in < num_inputs();++in){
    if (schedules[timeSlot][in] == -1){
     /// not matched in this time slot
      inputAvailability[in]++;
    }
  }

  for (int out = 0;out < num_outputs();++out){
    if (out_match[out] == -1){
     /// not matched
      outputAvailability[out]++;
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_HalfHalf_AvailabilityAware::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_HalfHalf_AvailabilityAware::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;
    assert(VOQLength[s][d] > 0);

    BST::update<int>(bstForSamplingAtInput[s], d + bstLeftStart, -1);
    --VOQLength[s][d];
  }
}

void SB_QPS_HalfHalf_AvailabilityAware::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
  }
  for (auto &counter : VOQLength){
    std::fill(counter.begin(), counter.end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_AvailabilityAware::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
  std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void SB_QPS_HalfHalf_AvailabilityAware::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_HalfHalf_AvailabilityAware



/**
 * Start of implementation for class SB_QPS_Adaptive
 */
SB_QPS_Adaptive::SB_QPS_Adaptive(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)),
  outputAvailability(_num_outputs, 0),
  inputAvailability(_num_inputs, 0)
  {
    bstLeftStart = BST::nearest_power_of_two(_num_outputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
    }
    _cf_rel_time = 0;
    /// generate initial schedulers (used for the first batch)
    for (auto &schedul: schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_Adaptive::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_Adaptive::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_Adaptive::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_Adaptive::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  /// maximum number of accepts for each time slots
  std::vector<int> maxAccepts(num_outputs(), 1);
  for(int out = 0;out < num_outputs();++out){
    maxAccepts[out] += int(std::round((double)outputAvailability[out] / (getBatchSize() - timeSlot)));
  }

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (maxAccepts[out] > 1 ){
      outAccepts[out].push_back(in);
    } else {/// normal QPS
      if(outAccepts[out].empty()){
        outAccepts[out].push_back(in);
      }
      else if (VOQLength[outAccepts[out].front()][out] < VOQLength[in][out]){
        outAccepts[out][0] = in;
      }
    }

  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures
  std::vector<int> out_match(num_outputs(), -1);

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()){
      continue;
    }

    /// Schedule input with the largest VOQ proposal for current time slot
    int in = outAccepts[out][0];
    inputMatchBitmap[in].set(timeSlot);
    ouputMatchBitmap[out].set(timeSlot);
    schedules[timeSlot][in] = out;
    out_match[out] = in;
    virtualDepartures.emplace_back(in, out);

    /// Schedule inputs with remaining VOQs proposals for earlier available time slots
    if (maxAccepts[out] > 1){
      /// sort proposals based on VOQ lengths
      std::sort(outAccepts[out].begin(), outAccepts[out].end(), [=](const int in1, const int in2){
        return VOQLength[in1][out] > VOQLength[in2][out];
      });

      for (int i = 1; i < std::min(maxAccepts[out], (int)outAccepts[out].size()) && outputAvailability[out] > 0; ++i){
        int in = outAccepts[out][i];
        if (inputAvailability[in] == 0){
          continue;
        }

        /// available only when both available
        auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);
        for (int f = (int) (timeSlot) - 1; f >= 0; --f){
          if (!isAlreadyMatched.test(f)){
            inputMatchBitmap[in].set(f);
            ouputMatchBitmap[out].set(f);
            --inputAvailability[in];
            --outputAvailability[out];
            assert(schedules[f][in] == -1);
            schedules[f][in] = out;
            virtualDepartures.emplace_back(in, out);
            break;/// pay special attention (first fit)
          }
        }
      }
    }
  }

  for (int in = 0;in < num_inputs();++in){
    if (schedules[timeSlot][in] == -1){
      /// not matched in this time slot
      inputAvailability[in]++;
    }
  }

  for (int out = 0;out < num_outputs();++out){
    if (out_match[out] == -1){
      /// not matched
      outputAvailability[out]++;
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_Adaptive::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_Adaptive::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;
    assert(VOQLength[s][d] > 0);

    BST::update<int>(bstForSamplingAtInput[s], d + bstLeftStart, -1);
    --VOQLength[s][d];
  }
}

void SB_QPS_Adaptive::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
  }
  for (auto &counter : VOQLength){
    std::fill(counter.begin(), counter.end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_Adaptive::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
  std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void SB_QPS_Adaptive::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_Adaptive



/**
 * Start of implementation for class SB_QPS_HalfHalf_MI
 */
SB_QPS_HalfHalf_MI::SB_QPS_HalfHalf_MI(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)),
  outputAvailability(_num_outputs, 0),
  inputAvailability(_num_inputs, 0)
  {
  bstLeftStart = BST::nearest_power_of_two(_num_outputs);
  for (size_t i = 0; i < _num_inputs; ++i){
    bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
  }
  _cf_rel_time = 0;
  /// generate initial schedulers (used for the first batch)
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_MI::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_HalfHalf_MI::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_HalfHalf_MI::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_HalfHalf_MI::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  int maxIters = (((timeSlot + 1) * 2 > getBatchSize()) ? 3 : 1);
  for(int i = 0;i < maxIters;++i){
    qps(timeSlot);
  }
}
void SB_QPS_HalfHalf_MI::qps(size_t timeSlot){

  /// maximum number of accepts for each time slots
  int maxAccepts = (((timeSlot + 1) * 2 > getBatchSize())  ? 2 : 1);

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (maxAccepts > 1 ){
      outAccepts[out].push_back(in);
    } else {/// normal QPS
      if(outAccepts[out].empty()){
        outAccepts[out].push_back(in);
      }
      else if (VOQLength[outAccepts[out].front()][out] < VOQLength[in][out]){
        outAccepts[out][0] = in;
      }
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures
  std::vector<int> out_match(num_outputs(), -1);

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()){
      continue;
    }

    /// Schedule input with the largest VOQ proposal for current time slot
    int in = outAccepts[out][0];
    inputMatchBitmap[in].set(timeSlot);
    ouputMatchBitmap[out].set(timeSlot);
    schedules[timeSlot][in] = out;
    out_match[out] = in;
    virtualDepartures.emplace_back(in, out);

    /// Schedule inputs with remaining VOQs proposals for earlier available time slots
    if (maxAccepts > 1){
     /// sort proposals based on VOQ lengths
      std::sort(outAccepts[out].begin(), outAccepts[out].end(), [=](const int in1, const int in2){
        return VOQLength[in1][out] > VOQLength[in2][out];
      });

      for (int i = 1; i < std::min(maxAccepts, (int)outAccepts[out].size()) && outputAvailability[out] > 0; ++i){
        int in = outAccepts[out][i];
        if (inputAvailability[in] == 0){
          continue;
        }
        /// available only when both available
        auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);
        for (int f = (int) (timeSlot) - 1; f >= 0; --f){
          if (!isAlreadyMatched.test(f)){
            inputMatchBitmap[in].set(f);
            ouputMatchBitmap[out].set(f);
            --inputAvailability[in];
            --outputAvailability[out];
            assert(schedules[f][in] == -1);
            schedules[f][in] = out;
            virtualDepartures.emplace_back(in, out);
            break;/// pay special attention (first fit)
          }
        }
      }
    }
  }

  for (int in = 0;in < num_inputs();++in){
    if (schedules[timeSlot][in] == -1){
     /// not matched in this time slot
      inputAvailability[in]++;
    }
  }

  for (int out = 0;out < num_outputs();++out){
    if (out_match[out] == -1){
     /// not matched
      outputAvailability[out]++;
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_HalfHalf_MI::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_HalfHalf_MI::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;
    assert(VOQLength[s][d] > 0);

    BST::update<int>(bstForSamplingAtInput[s], d + bstLeftStart, -1);
    --VOQLength[s][d];
  }
}

void SB_QPS_HalfHalf_MI::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
  }
  for (auto &counter : VOQLength){
    std::fill(counter.begin(), counter.end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_MI::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
  std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void SB_QPS_HalfHalf_MI::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}



/**
 * Start of implementation for class SB_QPS_ThreeThird_MI
 */
SB_QPS_ThreeThird_MI::SB_QPS_ThreeThird_MI(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)),
  outputAvailability(_num_outputs, 0),
  inputAvailability(_num_inputs, 0)
  {
    bstLeftStart = BST::nearest_power_of_two(_num_outputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
    }
    _cf_rel_time = 0;
    /// generate initial schedulers (used for the first batch)
    for (auto &schedul: schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_ThreeThird_MI::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_ThreeThird_MI::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_ThreeThird_MI::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_ThreeThird_MI::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  int maxIters = 1;

  if((timeSlot + 1) * 1.5 > batchSize){
    maxIters = 3;
  } else if ((timeSlot + 1) * 2 > batchSize){
    maxIters = 2;
  }

  for(int i = 0;i < maxIters;++i){
    qps(timeSlot);
  }
}
void SB_QPS_ThreeThird_MI::qps(size_t timeSlot){

  /// maximum number of accepts for each time slots
  int maxAccepts = 1;
  if((timeSlot + 1) * 1.5 > batchSize){
    maxAccepts = 4;
  } else if ((timeSlot + 1) * 2 > batchSize){
    maxAccepts = 2;
  }

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (maxAccepts > 1 ){
      outAccepts[out].push_back(in);
    } else {/// normal QPS
      if(outAccepts[out].empty()){
        outAccepts[out].push_back(in);
      } else if (VOQLength[outAccepts[out].front()][out] < VOQLength[in][out]){
        outAccepts[out][0] = in;
      }
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures
  std::vector<int> out_match(num_outputs(), -1);

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()){
      continue;
    }
    
    /// Schedule input with the largest VOQ proposal for current time slot
    int in = outAccepts[out][0];
    inputMatchBitmap[in].set(timeSlot);
    ouputMatchBitmap[out].set(timeSlot);
    schedules[timeSlot][in] = out;
    out_match[out] = in;
    virtualDepartures.emplace_back(in, out);

    /// Schedule inputs with remaining VOQs proposals for earlier available time slots
    if (maxAccepts > 1){
      /// sort proposals based on VOQ lengths
      std::sort(outAccepts[out].begin(), outAccepts[out].end(), [=](const int in1, const int in2){
        return VOQLength[in1][out] > VOQLength[in2][out];
      });

      for (int i = 1; i < std::min(maxAccepts, (int)outAccepts[out].size()) && outputAvailability[out] > 0; ++i){
        int in = outAccepts[out][i];
        if (inputAvailability[in] == 0){
          continue;
        }
        /// available only when both available
        auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);
        for (int f = (int) (timeSlot) - 1; f >= 0; --f){
          if (!isAlreadyMatched.test(f)){
            inputMatchBitmap[in].set(f);
            ouputMatchBitmap[out].set(f);
            --inputAvailability[in];
            --outputAvailability[out];
            assert(schedules[f][in] == -1);
            schedules[f][in] = out;
            virtualDepartures.emplace_back(in, out);
            break;/// pay special attention (first fit)
          }
        }
      }
    }
  }

  for (int in = 0;in < num_inputs();++in){
    if (schedules[timeSlot][in] == -1){
      /// not matched in this time slot
      inputAvailability[in]++;
    }
  }

  for (int out = 0;out < num_outputs();++out){
    if (out_match[out] == -1){
      /// not matched
      outputAvailability[out]++;
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_ThreeThird_MI::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_ThreeThird_MI::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;
    assert(VOQLength[s][d] > 0);

    BST::update<int>(bstForSamplingAtInput[s], d + bstLeftStart, -1);
    --VOQLength[s][d];
  }
}

void SB_QPS_ThreeThird_MI::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
  }
  for (auto &counter : VOQLength){
    std::fill(counter.begin(), counter.end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_ThreeThird_MI::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
  std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void SB_QPS_ThreeThird_MI::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_ThreeThird_MI


/**
 * Start of implementation for class SB_QPS_HalfHalf_MA
 */
SB_QPS_HalfHalf_MA::SB_QPS_HalfHalf_MA(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)),
  outputAvailability(_num_outputs, 0),
  inputAvailability(_num_inputs, 0)
  {
    bstLeftStart = BST::nearest_power_of_two(_num_outputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
    }
    _cf_rel_time = 0;
    /// generate initial schedulers (used for the first batch)
    for (auto &schedul: schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_HalfHalf_MA::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_HalfHalf_MA::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_HalfHalf_MA::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_HalfHalf_MA::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);
  int maxIters = 1;
  for(int i = 0;i < maxIters;++i){
    qps(timeSlot);
  }
}
void SB_QPS_HalfHalf_MA::qps(size_t timeSlot){

  /// maximum number of accepts for each time slots
  int maxAccepts = (((timeSlot + 1) * 2 > getBatchSize())  ? 2 : 1);

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i) inputs[i] = i;
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (maxAccepts > 1 ){
      outAccepts[out].push_back(in);
    } else {/// normal QPS
      if(outAccepts[out].empty()){
        outAccepts[out].push_back(in);
      }
      else if (VOQLength[outAccepts[out].front()][out] < VOQLength[in][out]){
        outAccepts[out][0] = in;
      }
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures
  std::vector<int> out_match(num_outputs(), -1);

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()){
      continue;
    }

    /// sort proposals based on VOQ lengths
    std::sort(outAccepts[out].begin(), outAccepts[out].end(), [=](const int in1, const int in2){
      return VOQLength[in1][out] > VOQLength[in2][out];
    });

    for (int i = 0; i < std::min(maxAccepts, (int) outAccepts[out].size()) && outputAvailability[out] > 0; ++i){
      int in = outAccepts[out][i];
      if (inputAvailability[in] == 0 || VOQLength[in][out] == 0){
        continue;/// no available ts or VOQ is empty
      }
      /// available only when both available
      auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);
      for (auto f = (int) (timeSlot); f >= 0; --f){
        if (!isAlreadyMatched.test(f)){
          inputMatchBitmap[in].set(f);
          ouputMatchBitmap[out].set(f);
          --inputAvailability[in];
          --outputAvailability[out];
          --VOQLength[in][out];
          assert(schedules[f][in] == -1);
          schedules[f][in] = out;
          if (f == timeSlot) out_match[out] = in;
          virtualDepartures.emplace_back(in, out);
        }
      }
    }
  }

  for (int in = 0;in < num_inputs();++in){
    if (schedules[timeSlot][in] == -1){
      /// not matched in this time slot
      inputAvailability[in]++;
    }
  }

  for (int out = 0;out < num_outputs();++out){
    if (out_match[out] == -1){
      /// not matched
      outputAvailability[out]++;
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_HalfHalf_MA::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_HalfHalf_MA::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;

    //assert(VOQLength[s][d] > 0);
    BST::update<int>(bstForSamplingAtInput[s], d + bstLeftStart, -1);
  }
}

void SB_QPS_HalfHalf_MA::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
  }
  for (auto &counter : VOQLength){
    std::fill(counter.begin(), counter.end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_MA::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
  std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void SB_QPS_HalfHalf_MA::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_HalfHalf_MA



/**
 * Start of implementation for class SB_QPS_HalfHalf_MA_MI
 */
SB_QPS_HalfHalf_MA_MI::SB_QPS_HalfHalf_MA_MI(std::string name, int _num_inputs, int _num_outputs, int batchSize,
  std::mt19937::result_type seed) :
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed),
  eng(seed),
  bstForSamplingAtInput(_num_inputs),
  inputMatchBitmap(_num_inputs),
  ouputMatchBitmap(_num_outputs),
  VOQLength(_num_inputs, std::vector<int>(_num_outputs, 0)),
  outputAvailability(_num_outputs, 0),
  inputAvailability(_num_inputs, 0)
  {
    bstLeftStart = BST::nearest_power_of_two(_num_outputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      bstForSamplingAtInput[i].resize(2 * bstLeftStart, 0);
    }
    _cf_rel_time = 0;
    /// generate initial schedulers (used for the first batch)
    for (auto &schedul: schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_HalfHalf_MA_MI::init(const IQSwitch *iqSwitch){
 /// reserved for future use
}

void SB_QPS_HalfHalf_MA_MI::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert (iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert (srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);

    BST::update<int>(bstForSamplingAtInput[srcDestPair.first], srcDestPair.second + bstLeftStart);
    ++VOQLength[srcDestPair.first][srcDestPair.second];
  }
}

void SB_QPS_HalfHalf_MA_MI::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);

  /// copy out scheduler for the last batch
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  /// handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(iqSwitch, timeSlot);

  /// reset bit map, if it is the last time slot of a batch
  if (timeSlot == batchSize - 1){
    resetBitmap();
  }

  ++_cf_rel_time;
}

void SB_QPS_HalfHalf_MA_MI::qps(const saber::IQSwitch *iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  int maxIters = (((timeSlot + 1) * 2 > getBatchSize()) ? 3 : 1);
  for(int i = 0;i < maxIters;++i){
    qps(timeSlot);
  }
}
void SB_QPS_HalfHalf_MA_MI::qps(size_t timeSlot){

  /// maximum number of accepts for each time slots
  int maxAccepts = (((timeSlot + 1) * 2 > getBatchSize())  ? 2 : 1);

  std::vector<std::vector<int>> outAccepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i){
    inputs[i] = i;
  }
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
      continue;/// no packets
    }
    auto out = sampleOutputForInput(in);/// sample an output for this input
    assert(VOQLength[in][out] > 0);

    if (maxAccepts > 1 ){
      outAccepts[out].push_back(in);
    } else {/// normal QPS
      if(outAccepts[out].empty()){
        outAccepts[out].push_back(in);
      }
      else if (VOQLength[outAccepts[out].front()][out] < VOQLength[in][out]){
        outAccepts[out][0] = in;
      }
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures;/// virtual departures
  std::vector<int> out_match(num_outputs(), -1);

  /// Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
    if (outAccepts[out].empty()){
      continue;
    }

    /// sort proposals based on VOQ lengths
    std::sort(outAccepts[out].begin(), outAccepts[out].end(), [=](const int in1, const int in2){
      return VOQLength[in1][out] > VOQLength[in2][out];
    });

    for (int i = 0; i < std::min(maxAccepts, (int)outAccepts[out].size()) && outputAvailability[out] > 0; ++i){
      int in = outAccepts[out][i];
      if (inputAvailability[in] == 0 || VOQLength[in][out] == 0){
        continue;/// no available ts or VOQ is empty
      }
      /// available only when both available
      auto isAlreadyMatched = (inputMatchBitmap[in] | ouputMatchBitmap[out]);
      for (int f = (int) (timeSlot) ; f >= 0; --f){
        if (!isAlreadyMatched.test(f)){
          inputMatchBitmap[in].set(f);
          ouputMatchBitmap[out].set(f);
          --inputAvailability[in];
          --outputAvailability[out];
          --VOQLength[in][out];
          assert(schedules[f][in] == -1);
          schedules[f][in] = out;
          if(f == timeSlot){
            out_match[out] = in;
          }
          virtualDepartures.emplace_back(in, out);
        }
      }
    }
  }

  for (int in = 0;in < num_inputs();++in){
    if (schedules[timeSlot][in] == -1){
      /// not matched in this time slot
      inputAvailability[in]++;
    }
  }

  for (int out = 0;out < num_outputs();++out){
    if (out_match[out] == -1){
      /// not matched
      outputAvailability[out]++;
    }
  }

  #ifdef DEBUG
    std::cerr << VOQLength << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif

  handlePacketDepartures(virtualDepartures);
}

int SB_QPS_HalfHalf_MA_MI::sampleOutputForInput(int input){
  assert (input >= 0 && input < _num_inputs);

  std::uniform_real_distribution<double> dist(0, bstForSamplingAtInput[input][1]);
  double r = dist(eng);
  int out = BST::upper_bound<int>(bstForSamplingAtInput[input], r) - bstLeftStart;

  #ifdef DEBUG
    std::cerr << "random : " << r << "\n";
    std::cerr << "in : " << input << "\n";
    std::cerr << "out : " << out << "\n";
    std::cerr << "bst : " << bstForSamplingAtInput[input] << "\n";
    std::cerr << "VOQ[i][j] : " << VOQLength[input][out] << "\n";
  #endif

  assert (out >= 0 && out < _num_outputs);
  return out;
}

void SB_QPS_HalfHalf_MA_MI::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;

    //assert(VOQLength[s][d] > 0);
    BST::update<int>(bstForSamplingAtInput[s], d + bstLeftStart, -1);
  }
}

void SB_QPS_HalfHalf_MA_MI::reset(){
  BatchScheduler::reset();
  resetBitmap();
  _cf_rel_time = 0;
  for (size_t i = 0; i < _num_inputs; ++i){
    std::fill(bstForSamplingAtInput[i].begin(), bstForSamplingAtInput[i].end(), 0);
  }
  for (auto &counter : VOQLength){
    std::fill(counter.begin(), counter.end(), 0);
  }
  for (auto &schedul: schedules){
    std::fill(schedul.begin(), schedul.end(), -1);
  }
}

void SB_QPS_HalfHalf_MA_MI::resetBitmap(){
  for (auto &matchBitmap : inputMatchBitmap){
    matchBitmap.reset();
  }
  for (auto &matchBitmap : ouputMatchBitmap){
    matchBitmap.reset();
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
  std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void SB_QPS_HalfHalf_MA_MI::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n" << "bst: " << bstForSamplingAtInput << "\n";
}

///< End of implementation for class SB_QPS_HalfHalf_MA_MI

/**
 * Start of implementation for class SB_QPS_Conditional
 */
const int SB_QPS_Conditional::NULL_SCHEDULE = -1;

SB_QPS_Conditional::SB_QPS_Conditional(std::string name, int _num_inputs, int _num_outputs, int iter_round, int batchSize, 
  std::mt19937::result_type seed): 
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
  seed(seed), 
  eng(seed), 
  inputMatchBitmap(_num_inputs),
  outputMatchBitmap(_num_outputs), 
  numIterations(iter_round),
  conditionalApprovals(batchSize, std::vector<int>(_num_outputs, NULL_SCHEDULE)),
  inputQueues(_num_inputs, 
  Queue_Sampler(_num_outputs)),
  outputAvailability(_num_outputs, batchSize), 
  inputAvailability(_num_inputs, batchSize) 
  {
    _cf_rel_time = 0;

    // generate initial schedulers (used for the first frame)
    for (auto &schedul : schedules){
      std::fill(schedul.begin(), schedul.end(), -1);
    }
  }

void SB_QPS_Conditional::init(const IQSwitch *iqSwitch){
  // reserved
}

void SB_QPS_Conditional::schedule(const saber::IQSwitch *iqSwitch){
  auto timeSlot = (_cf_rel_time % batchSize);
  // copy out scheduler for the last frame
  std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(),
            _in_match.begin());
  std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), -1);

  assert(isBatchSizeFixed);

  // handle arrivals
  handlePacketArrivals(iqSwitch);

  qps(timeSlot, 1);

  // extra fix and other rounds at last frame in batch
  if (timeSlot == batchSize - 1){
      endOfBatchOptimization();
      for (int r=2; r <= numIterations; ++r){
          for (int f=0; f < (int)batchSize; ++f){
              qps(f, r);
          }
          endOfBatchOptimization();
      }

      resetBitmap();  // prepare for next batch
  }
  ++_cf_rel_time;
}

void SB_QPS_Conditional::handlePacketArrivals(const IQSwitch *iqSwitch){
  assert(iqSwitch != nullptr);
  auto arrivals = iqSwitch->get_arrivals();
  for (const auto &srcDestPair : arrivals){
    if (srcDestPair.first == -1){
      break;
    }
    assert(srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
    assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);
    inputQueues[srcDestPair.first].add_item(srcDestPair.second);
  }
}

void SB_QPS_Conditional::qps(size_t timeSlot, int iteration){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  std::vector<Linear_Priority_Queue> out_accepts(_num_outputs, Linear_Priority_Queue{2});

  // shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i)
      inputs[i] = i;
  std::shuffle(inputs.begin(), inputs.end(), eng);

  // Step 1: Proposing
  for (auto in: inputs){
    if (getQueueLengthForInput(in) == 0 || inputMatchBitmap[in].test(timeSlot))
      continue; // no packets or already matched
    Queue_Sampler::Sample_Type tp =
        (iteration > 1) ? Queue_Sampler::UNIFORM : Queue_Sampler::QPS;
    auto out =
        inputQueues[in].sample(eng, tp); // sampling an output for this input
    assert(inputQueues[in].get_packet_number(out) > 0);

    out_accepts[out].insert(in, inputQueues[in].get_packet_number(out));
  }

    std::vector<std::pair<int, int>> virtualDepartures; // virtual departures

  // Step 2: Accept
  for (int out = 0; out < _num_outputs; ++out){
      // first rank proposal: match into schedule
      int in = out_accepts[out].queue[0].first;
      if (in == out_accepts[out].null_symbol) continue;

      if (!outputMatchBitmap[out].test(timeSlot)){
          match(in, out, timeSlot);
          // save second proposal to conditional acceptance
          in = out_accepts[out].queue[1].first;
          if (in == out_accepts[out].null_symbol) continue;
      }
      conditionalApprovals[timeSlot][out] = in;
  }
  #ifdef DEBUG
    std::cerr << inputQueues << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif
  handlePacketDepartures(virtualDepartures);
}

void SB_QPS_Conditional::match(int input, int output, int timeSlot){
    inputMatchBitmap[input].set(timeSlot);
    outputMatchBitmap[output].set(timeSlot);
    --inputAvailability[input];
    --outputAvailability[output];
    assert(schedules[timeSlot][input] == -1);
    schedules[timeSlot][input] = output;
    inputQueues[input].remove_item(output);
}

void SB_QPS_Conditional::endOfBatchOptimization(){

    std::vector<std::pair<int, int>> virtualDepartures;

    // shuffle the output ports
    std::vector<int> outputs(_num_outputs, 0);
    for (int i = 0; i < _num_inputs; ++i)
      outputs[i] = i;
    std::shuffle(outputs.begin(), outputs.end(), eng);

    for (auto out:outputs){
        std::vector<int> approvalPorts, freeTimeSlots;
        // sequence of extra fix

        for(int t=0; t<(int)batchSize; ++t){
            if (!outputMatchBitmap[out].test(t)){
                freeTimeSlots.push_back(t);
            }
        }

        for (int t=0; t<(int)batchSize; ++t){
            int in = conditionalApprovals[t][out];
            if (in != NULL_SCHEDULE){
              approvalPorts.push_back(in);
            }
        }
        if (approvalPorts.empty()) continue;
        std::shuffle(approvalPorts.begin(), approvalPorts.end(), eng);

        size_t extraFreeTime = freeTimeSlots.size() - approvalPorts.size();
        int approvalCount = (int) approvalPorts.size();
        approvalPorts.reserve(freeTimeSlots.size());
        auto begin_ptr = approvalPorts.begin(); // IMPORTANT: pointers are invalidated after resize
        std::uniform_int_distribution<int> dist(0, approvalCount-1);
        for (int i=0; i<(int)extraFreeTime; ++i){
            int sample = dist(eng);
            assert(*(begin_ptr+sample)>=0 && *(begin_ptr+sample)<_num_inputs);
            approvalPorts.push_back(*(begin_ptr+sample));
        }

        assert(approvalPorts.size()>=freeTimeSlots.size());
        for (int app=0; app<(int)freeTimeSlots.size(); ++app){
            int in = approvalPorts[app], t = freeTimeSlots[app];
            if (!inputMatchBitmap[in].test(t) && inputQueues[in].get_packet_number(out)>0){
                match(in, out, t);
            }
        }
    }

    handlePacketDepartures(virtualDepartures);
}

void SB_QPS_Conditional::handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures){
  for (const auto &srcDestPair : virtualDepartures){
    auto s = srcDestPair.first;
    auto d = srcDestPair.second;
    inputQueues[s].remove_item(d);
  }
}

void SB_QPS_Conditional::resetBitmap(){
  for (auto &mf : inputMatchBitmap){
    mf.reset();
  }
  for (auto &mf : outputMatchBitmap){
    mf.reset();
  }
  for (auto &cond : conditionalApprovals)
  {
    std::fill(cond.begin(), cond.end(), NULL_SCHEDULE);
  }
  std::fill(inputAvailability.begin(), inputAvailability.end(), batchSize);
  std::fill(outputAvailability.begin(), outputAvailability.end(), batchSize);
}

void SB_QPS_Conditional::reset(){
  BatchScheduler::reset();
  resetBitmap();
  for (auto &sampler: inputQueues)
    sampler.reset();
  _cf_rel_time = 0;
  for (auto &sched : schedules){
    std::fill(sched.begin(), sched.end(), -1);
  }
}

void SB_QPS_Conditional::display(std::ostream &os) const {
  BatchScheduler::display(os);
  os << "---------------------------------------------------------------------\n";
  os << "seed: " << seed << "\n";
  for (const auto &sampler: inputQueues)
  {
    os << sampler;
  }
  os << "\n";
}
///< End of implementation for class SB_QPS_Conditional


/**
 * Start of implementation for class SB_QPS_Fixed
 */
const int SB_QPS_Fixed::NULL_SCHEDULE = -1;

SB_QPS_Fixed::SB_QPS_Fixed(std::string name, int _num_inputs, int _num_outputs, int batchSize,
	std::mt19937::result_type seed, int cutoff, bool debug_mode): 
  BatchScheduler(name, _num_inputs, _num_outputs, batchSize, true),
	seed(seed), 
  eng(seed), 
  inputMatchBitmap(_num_inputs),
	outputMatchBitmap(_num_outputs), 
  _debug_mode(debug_mode), 
  maxAccepts(cutoff),
	outputAvailability(_num_outputs, batchSize), 
  inputAvailability(_num_inputs, batchSize) 
  { 
    _cf_rel_time = 0;

    inputQueues.reserve(_num_inputs);
    for (size_t i = 0; i < _num_inputs; ++i){
      inputQueues.emplace_back(_num_outputs);
    }

    // generate initial schedulers (used for the first frame)
    for (auto& sched : schedules){
      std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
    }
    for (auto& sched : schedules_pre){
      std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
    }

    if(_debug_mode){
        _debug_stream.open("debug_dump.txt", std::ofstream::out);
        _debug_stream << "scheduling starts\n";
    }
    else{
          _debug_stream.setstate(std::ofstream::badbit);
          // disable _debug_stream
    }
  }

void SB_QPS_Fixed::init(const IQSwitch* iqSwitch){
	// reserved
}

void SB_QPS_Fixed::handlePacketArrivals(const IQSwitch* iqSwitch){
	assert(iqSwitch != nullptr);

	auto arrivals = iqSwitch->get_arrivals();
	_debug_stream << "handle arrivals\n";

	for (const auto& srcDestPair : arrivals){
		if (srcDestPair.first == -1){
			break;
    }
		assert(srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
		assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);
		inputQueues[srcDestPair.first].add_item(srcDestPair.second);
		_debug_stream << "(" << srcDestPair.first << ", " << srcDestPair.second << ")";
	}
	_debug_stream << "\n";
}

void SB_QPS_Fixed::schedule(const saber::IQSwitch* iqSwitch){
	auto timeSlot = (_cf_rel_time % batchSize);
  _debug_stream << "Round: " << std::setw(6) << _cf_rel_time << "\tframe " << std::setw(3) << timeSlot << "\n";

  // copy schedule_pre to _in_match, output to simulator
  std::copy(schedules_pre[timeSlot].begin(), schedules_pre[timeSlot].end(), _in_match.begin());

  // clear the schedule_pre
  std::fill(schedules_pre[timeSlot].begin(), schedules_pre[timeSlot].end(), NULL_SCHEDULE);

  // handle arrivals
  handlePacketArrivals(iqSwitch);

	qps(iqSwitch, timeSlot);

	_debug_stream << "buffer state\n" <<std::setw(11) << " ";
	for (int f=0; f<batchSize; ++f){
    _debug_stream << std::setw(3) << f;
  }
	_debug_stream << "\n";
	for (size_t f=0; f<batchSize; ++f){
	    _debug_stream << "frame " << std::setw(3) << f << ": ";
	    for (int out_port:schedules[f]){
            _debug_stream << std::setw(3) << out_port;
	    }
	    _debug_stream << "\n";
	}

	if (timeSlot == batchSize-1){ //this batch ends
		///schedules_pre should be empty now
		if (_debug_mode)
		{
			for (auto schedule:schedules_pre)
			{
				for (auto cell: schedule)
				{
					assert(cell == NULL_SCHEDULE);
				}
			}
		}
		std::swap(schedules, schedules_pre);

		/// reset availability for _schedule
		resetBitmap();
	}
	++_cf_rel_time;
}

void SB_QPS_Fixed::qps(const saber::IQSwitch* iqSwitch, size_t timeSlot){
  /// Refer section 3 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

  assert(isBatchSizeFixed);

  _debug_stream << "queue state\n" << std::setw(11) << " ";
  for (int f=0; f<_num_inputs; ++f)
  {
      _debug_stream << std::setw(3) << f;
  }
  _debug_stream << "\n";
  for (size_t in=0; in<_num_inputs; ++in){
      _debug_stream << "input " << std::setw(3) << in << ": " << inputQueues[in] << "\n";
  }

  /// proposals recieved by each output port
  std::vector<std::vector<int>> out_accepts(num_outputs());

  /// shuffle inputs
  std::vector<int> inputs(_num_inputs, 0);
  for (int i = 0; i < _num_inputs; ++i)
          inputs[i] = i;
  std::shuffle(inputs.begin(), inputs.end(), eng);

  /// Step 1: Proposing
  for (int i = 0; i < _num_inputs; ++i){
    int in = inputs[i];
    if (getQueueLengthForInput(in) == 0){
            continue;  /// no packets
    }

    int out = inputQueues[in].sample(eng, Queue_Sampler::QPS); /// sampling an output for this input
    assert(inputQueues[in].get_packet_number(out) > 0);

    if(out_accepts[out].size()<maxAccepts){  /// only accepts up to maxAccepts proposals
      out_accepts[out].push_back(in);
    }
  }

  std::vector<std::pair<int, int>> virtualDepartures; // virtual departures
  std::vector<int> out_match(num_outputs(), NULL_SCHEDULE); /// matching for output ports

  /// Step 2: Accept
  _debug_stream << "accepting\n";
  for (int out = 0; out < _num_outputs; ++out){
      if (out_accepts[out].empty()){  // no proposal received
        continue;
      }

      /// Sort proposals based on VOQ lengths actually we can optimize by move sorting inside the if check
      /// and whenever if fails, we just find the largest element and move it to the beginning of the vector!
      std::sort(out_accepts[out].begin(), out_accepts[out].end(), [=](const int in1, const int in2){
        return inputQueues[in1].get_packet_number(out) > inputQueues[in2].get_packet_number(out);
      });

      for (int in: out_accepts[out]){
        if (outputAvailability[out] == 0){
            if(_debug_mode){
                _debug_stream << "(" <<  in << ", " << out << ", -)" ;
                continue;
            } else {
                break;
            }
        }
        _debug_stream << "(" <<  in << ", " << out << ", ";

        if (inputAvailability[in] == 0){
          continue; // no available ts
        }

        /// available only when both available
        auto mf = (inputMatchBitmap[in] | outputMatchBitmap[out]);
        bool success = false;
        for (int f=0; f<(int)batchSize; ++f){
            if (!mf.test(f)){
                inputMatchBitmap[in].set(f);
                outputMatchBitmap[out].set(f);
                --inputAvailability[in];
                --outputAvailability[out];
                assert(schedules[f][in] == NULL_SCHEDULE);
                schedules[f][in] = out;
                virtualDepartures.emplace_back(in, out);
                _debug_stream << f << ")";
                success = true;
                break; // pay special attention (first fit)
            }
        }
        if (!success)
            _debug_stream << "-)";
      }
  }
  _debug_stream << "\n";

  #ifdef DEBUG
    std::cerr << inputQueues << "\n";
    std::cerr << virtualDepartures << "\n\n";
  #endif
	handlePacketDepartures(virtualDepartures);
}

void SB_QPS_Fixed::handlePacketDepartures(const std::vector<std::pair<int, int>>& virtualDepartures){
	for (const auto& srcDestPair : virtualDepartures){
		assert(inputQueues[srcDestPair.first].get_packet_number(srcDestPair.second) > 0);
		inputQueues[srcDestPair.first].remove_item(srcDestPair.second);
	}
}

void SB_QPS_Fixed::resetBitmap(){
	for (auto& mf : inputMatchBitmap){
		mf.reset();
  }
	for (auto& mf : outputMatchBitmap){
		mf.reset();
  }
	std::fill(inputAvailability.begin(), inputAvailability.end(), batchSize);
	std::fill(outputAvailability.begin(), outputAvailability.end(), batchSize);
}

void SB_QPS_Fixed::reset(){
	BatchScheduler::reset();
	resetBitmap();
	std::fill(inputAvailability.begin(), inputAvailability.end(), batchSize);
	std::fill(outputAvailability.begin(), outputAvailability.end(), batchSize);
	for (size_t i = 0; i < _num_inputs; ++i){
		inputQueues[i].reset();
  }
	_cf_rel_time = 0;
	for (auto& sched : schedules){
    std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	}
	for (auto& sched : schedules_pre){
    std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	}
}

void SB_QPS_Fixed::display(std::ostream& os) const {
	BatchScheduler::display(os);
	os << "---------------------------------------------------------------------\n";
	os << "seed: " << seed << "\n input queues\n";
	for (const auto & item : inputQueues){
		os << item << "\n";
	}
}
///< End of implementation for class SB_QPS_Fixed

} ///< namespace saber