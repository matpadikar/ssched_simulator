//
#include "scheduler_factory.h"
#include "sliding_window_qps.h"
#include <exceptions.hpp>

namespace saber {
// Implementation of Create for class SchedulerFactory
//  //////////////////////////////////////////////////////////
Scheduler *SchedulerFactory::Create(const json &conf) {
  if (!(conf.count("name")))
    throw MissingArgumentException("Argument conf MUST contain name!");
  if (!(conf.count("num_inputs")))
    throw MissingArgumentException("Argument conf MUST contain num_inputs!");
  if (!(conf.count("num_outputs")))
    throw MissingArgumentException("Argument conf MUST contain num_inputs!");
  std::string name = conf["name"].get<std::string>();
  if (name.empty())
    throw EmptyNameException(
        "Argument conf MUST contain a key \"name\", whose value is NOT EMPTY!");

  int num_inputs = conf["num_inputs"].get<int>();
  int num_outputs = conf["num_outputs"].get<int>();
  bool out_match_enabled =
      (conf.count("out_match_enabled") ? conf["out_match_enabled"].get<bool>()
                                       : false);

  Scheduler *sched = nullptr;
  if (name == "dummy") {
    sched = new DummyScheduler(name, num_inputs, num_inputs, out_match_enabled);
  } else if (name == "maximum_weight") {
    sched =
        new MaxWeightScheduler(name, num_inputs, num_inputs, out_match_enabled);
  } else if (name == "randomized_maximal") {
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new RandomizedMaximalScheduler(name, num_inputs, num_outputs, seed);
  } /* else if (name == "round_robin") {
    std::vector<int> initial_match(num_inputs, -1);
    for (int i = 0; i < num_inputs; ++i) initial_match[i] = i % num_outputs;
    if (conf.count("initial_matching")) initial_match =
  conf["initial_matching"].get<std::vector<int>>(); sched = new RoundRobin(name,
  num_inputs, num_outputs, initial_match);
  } */
  else if (name == "greedy_maximal") {
    // TODO
  } else if (name == "serena") {
    // TODO
  } else if (name == "c-serenade") {
    // TODO
  } else if (name == "o-serenade") {
    // TODO
  } else if (name == "islip") {
    auto iterations =
        int(std::ceil(std::log2(std::max(num_inputs, num_outputs))));
    std::vector<int> grant_pointer(num_outputs, 0);
    std::vector<int> accept_pointer(num_inputs, 0);
    if (conf.count("iterations"))
      iterations = conf["iterations"].get<int>();
    if (conf.count("grant_pointer"))
      grant_pointer = conf["grant_pointer"].get<std::vector<int>>();
    if (conf.count("accept_pointer"))
      accept_pointer = conf["accept_pointer"].get<std::vector<int>>();
    sched = new iSLIP(name, num_inputs, num_outputs, iterations, grant_pointer,
                      accept_pointer);
  } /*else if (name == "slqf"){
    sched = new sLQF(name, num_inputs, num_outputs);
  } else if ( name == "ipoc" ) {
    int iteration = (conf.count("iterations")?conf["iterations"].get<int>():1);
    sched = new iPOC(name, num_inputs, num_outputs, iteration);
  }*/
  else if (name == "ilqf") {
    // TODO
  } else if (name == "pim") {
    // TODO
  } else if (name == "qps") {
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    int iterations = 1;
    bool without_replacement = false;
    std::string accept_policy{"longest_first"};
    if (conf.count("seed"))
      seed = conf["seed"].get<unsigned>();
    if (conf.count("iterations"))
      iterations = conf["iterations"].get<int>();
    if (conf.count("without_replacement"))
      without_replacement = conf["without_replacement"].get<bool>();
    if (conf.count("accept_policy"))
      accept_policy = conf["accept_policy"].get<std::string>();

    sched =
        new QPS(name + "|" + accept_policy + "|" + std::to_string(iterations) +
                    "|" + std::to_string(without_replacement),
                num_inputs, num_outputs, seed, iterations, accept_policy,
                without_replacement);

  } else if (name == "randomized_edge_coloring") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>()
                                  : (10 * num_inputs * num_outputs));
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new RandomizedEdgeColoringCAggarwal(name, num_inputs, num_outputs,
                                                frame_size, seed);
  } else if (name == "sb_qps_half_half_oblivious") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new SB_QPS_HalfHalf_Oblivious(name, num_inputs, num_outputs,
                                          frame_size, seed);
  } else if (name == "sb_qps_half_half_availability_aware") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new SB_QPS_HalfHalf_AvailabilityAware(name, num_inputs, num_outputs,
                                                  frame_size, seed);
  } else if (name == "sb_qps_half_half_mi") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched =
        new SB_QPS_HalfHalf_MI(name, num_inputs, num_outputs, frame_size, seed);
  } else if (name == "sb_qps_three_third_mi") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new SB_QPS_ThreeThird_MI(name, num_inputs, num_outputs, frame_size,
                                     seed);
  } else if (name == "sb_qps_half_half_ma") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched =
        new SB_QPS_HalfHalf_MA(name, num_inputs, num_outputs, frame_size, seed);
  } else if (name == "sb_qps_half_half_ma_mi") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new SB_QPS_HalfHalf_MA_MI(name, num_inputs, num_outputs, frame_size,
                                      seed);
  } else if (name == "sb_qps_adaptive") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched =
        new SB_QPS_Adaptive(name, num_inputs, num_outputs, frame_size, seed);
  } else if (name == "sb_qps_basic") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                         ? conf["seed"].get<unsigned>()
                         : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
    sched = new SB_QPS_Basic(name, num_inputs, num_outputs, frame_size, seed);
  } else if (name == "sliding_window_qps") {
  int frame_size =
	  (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
  int cutoff =
	  (conf.count("cutoff") ? conf["cutoff"].get<int>() : num_inputs+num_outputs);
bool batching =
	  (conf.count("batching") ? conf["batching"].get<bool>() : false);
  unsigned seed = (conf.count("seed")
	  ? conf["seed"].get<unsigned>()
	  : static_cast<unsigned>(
		  sys_clock_t::now().time_since_epoch().count()));
      bool debug_mode = conf.count("debug") && conf["debug"].get<bool>();
  sched = new Sliding_Window_QPS(name, num_inputs, num_outputs, frame_size, seed, cutoff, batching, debug_mode);
  } else if (name == "sb_qps_conditional") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                     ? conf["seed"].get<unsigned>()
                     : static_cast<unsigned>(
                         sys_clock_t::now().time_since_epoch().count()));
    int iter_round = (conf.count("iter_round") ? conf["iter_round"].get<int>() : 1);
    sched = new SB_QPS_Conditional(name, num_inputs, num_outputs, iter_round, frame_size, seed);
  }else if (name == "sw_qps_best_fit") {
      int frame_size =
              (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
      unsigned seed = (conf.count("seed")
                       ? conf["seed"].get<unsigned>()
                       : static_cast<unsigned>(
                               sys_clock_t::now().time_since_epoch().count()));
      bool debug_mode = conf.count("debug") && conf["debug"].get<bool>();
      sched = new SW_QPS_Best_Fit(name, num_inputs, num_outputs, frame_size, seed, debug_mode);
  }
  else if (name == "sb_qps_fixed") {
    int frame_size =
        (conf.count("frame_size") ? conf["frame_size"].get<int>() : 20);
    unsigned seed = (conf.count("seed")
                     ? conf["seed"].get<unsigned>()
                     : static_cast<unsigned>(
                         sys_clock_t::now().time_since_epoch().count()));
	int cutoff =
	  (conf.count("cutoff") ? conf["cutoff"].get<int>() : num_inputs+num_outputs);
    sched = new SB_QPS_Fixed(name, num_inputs, num_outputs, frame_size, seed, cutoff, debug_mode);

  }else {
    throw UnknownParameterException("Unknown scheduler name: " + name + "!");
  }
  return sched;
}
} // namespace saber
