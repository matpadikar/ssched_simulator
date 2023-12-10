

#ifndef INJECTION_MODEL_FACTORY_H
#define INJECTION_MODEL_FACTORY_H

#include <iostream>

#include "bernoulli_injection_model.h"
#include "injection_model.h"
#include "onoff_injection_model.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace saber {
class InjectionModelFactory {
public:
  static InjectionModel *Create(const json &conf);
};
} // namespace saber

#endif // INJECTION_MODEL_FACTORY_H
