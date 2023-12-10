//  simulator_factory.h header file

#ifndef SIMULATOR_FACTORY_H
#define SIMULATOR_FACTORY_H

#include "simulator.h"
#include <exceptions.hpp>

namespace saber {
class SimulatorFactory {
public:
  static Simulator *Create(const json &conf);
}; // class IQSwitchSimulator
} // namespace saber

#endif // SIMULATOR_FACTORY_H
