#include "batch_scheduler.h"
#include <switch/iq_switch.h>

namespace saber {

/**
 * Start of implementation for class BatchScheduler
 */

BatchScheduler::BatchScheduler(const std::string &name, 
  int numInputs, int numOutputs, int batchSize, bool isBatchSizeFixed):
  Scheduler(name, numInputs, numOutputs, false),
  isBatchSizeFixed(isBatchSizeFixed),
  initialBatchSize(static_cast<const size_t>(batchSize)),
  batchSize(static_cast<size_t>(batchSize)),
  _cf_rel_time(0),
  schedules(static_cast<unsigned long>(batchSize), std::vector<int>(numInputs, -1)),
  schedules_pre{},
  _pf_rel_time(0)
{ }

void BatchScheduler::reset() {
  _cf_rel_time = 0;
  _pf_rel_time = 0;
  batchSize = initialBatchSize;
  schedules_pre.clear();
  schedules.resize(batchSize, std::vector<int>(_num_inputs, -1));
}

void BatchScheduler::display(std::ostream &os) const {
  Scheduler::display(os);
  os << "---------------------------------------------------------------------------------\n";
  os << "BatchSize: " << getBatchSize() << "\n";
  os << "Is batch size fixed: " << getIsBatchSizeFixed() << "\n";
  os << "Schedules (previous batch): \n";
  int cnt = 1;
  for ( const auto& m : schedules_pre ){
    os << "Time Slot " << (cnt ++) << " : " << m << "\n";
  }
  os << "Schedules (current batch): \n";
  for ( const auto& m : schedules ){
    os << "Time Slot " << (cnt ++) << " : " << m << "\n";
  }
}

///< End of implementation for class BatchScheduler

} ///< namespace saber