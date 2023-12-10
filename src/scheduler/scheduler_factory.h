// scheduler_factory.h header file

#ifndef SCHEDULER_FACTORY_H
#define SCHEDULER_FACTORY_H

#include <chrono>
#include <cmath>

#include <nlohmann/json.hpp>

#include "batch_scheduler/batch_scheduler.h"
#include "edge_coloring_batch_scheduler.h"
#include "max_weight_scheduler.h"
#include "maximal_scheduler.h"
#include "miscellaneous.h"
#include "qps.h"
#include "batch_scheduler/sb_qps.h"
#include "scheduler.h"

namespace saber {
using json = nlohmann::json;
using sys_clock_t = std::chrono::system_clock;
// Class for scheduler factory
// ///////////////////////////////////////////////////////////////////////////////////////////////
class SchedulerFactory {
public:
  static Scheduler *Create(const json &conf);
}; // class SchedulerFactory

} // namespace saber

#endif // SCHEDULER_FACTORY_H
