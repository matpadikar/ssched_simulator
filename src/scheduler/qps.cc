
#include "qps.h"
#include <switch/iq_switch.h>

namespace saber {
//////////////////////////////
// Implementation of class QPS
//////////////////////////////
QPS::QPS(std::string name, int num_inputs, int num_outputs,
         std::mt19937::result_type seed, int iterations,
         std::string accept_policy, bool without_replacement)
    : RandomizedScheduler(std::move(name), num_inputs, num_outputs, true, seed),
      _iterations(iterations), _without_replacement(without_replacement),
      _accept_policy(std::move(accept_policy)),
      _bst(static_cast<unsigned long>(num_inputs)) {
  assert(iterations > 0 && "Iterations MUST be a positive integer");
  _left_start = static_cast<int>(BST::nearest_power_of_two(_num_outputs));
  for (size_t i = 0; i < num_inputs; ++i)
    _bst[i].resize(2 * _left_start, 0);
}

void QPS::reset() {
  RandomizedScheduler::reset();
  for (size_t i = 0; i < _num_inputs; ++i)
    std::fill(_bst[i].begin(), _bst[i].end(), 0);
}

void QPS::display(std::ostream &os) const {
  RandomizedScheduler::display(os);
  os << "------------------------------------------------------------------\n";
  os << "iterations           : "   << _iterations
     << "\naccepting policy     : " << _accept_policy
     << "\nwithout replacement  : " << _without_replacement
     << "\nbst                  : " << _bst << "\n";
}

void QPS::handleArrivals(const saber::IQSwitch *sw) {
  assert(sw != nullptr);
  auto arrivals = sw->get_arrivals();
  for (const auto &sd : arrivals) {
    if (sd.first == -1)
      break;
    assert(sd.first >= 0 && sd.first < _num_inputs);
    assert(sd.second >= 0 && sd.second < _num_outputs);
    BST::update<int>(_bst[sd.first], sd.second + _left_start);
  }
}

void QPS::handleDepartures(const saber::IQSwitch *sw) {
  assert(sw != nullptr);
  for (int s = 0; s < _num_inputs; ++s) {
    auto d = _in_match[s];

    if (d != -1 && sw->get_queue_length(s, d) > 0) {
      BST::update<int>(_bst[s], d + _left_start, -1);
    }
  }
}

int QPS::sampling(int source) {
  assert(source >= 0 && source < _num_inputs);
  std::uniform_real_distribution<double> dist(0, _bst[source][1]);
  double r = dist(_eng);

  int out = BST::upper_bound<int>(_bst[source], r) - _left_start;

#ifdef _DEBUG_QPS_
  std::cerr << "random : " << r << "\n";
  std::cerr << "out : " << out << "\n";
  std::cerr << "bst : " << _bst[source] << "\n";
#endif

  assert(out >= 0 && out < _num_outputs);
  return out;
}

int QPS::getQueueLength(int source) {
  assert(source >= 0 && source < _num_inputs);
  return _bst[source][1];
}

int QPS::removeVOQ(int source, int destination) {
  assert(source >= 0 && source < _num_inputs);
  assert(destination >= 0 && destination < _num_outputs);

  return BST::remove<int>(_bst[source], destination + _left_start);
}

void QPS::restoreVOQ(
    const std::vector<std::pair<std::pair<int, int>, int>> &restore_pairs) {
  for (const auto &sdq : restore_pairs) {
    assert(sdq.first.first >= 0 && sdq.first.first < _num_inputs &&
           "source port range check");
    assert(sdq.first.second >= 0 && sdq.first.second < _num_outputs &&
           "destination port range check");
    assert(sdq.second > 0 && "queue length");
    BST::update<int>(_bst[sdq.first.first], sdq.first.second + _left_start, sdq.second);
  }
}

void QPS::schedule(const saber::IQSwitch *sw) {
  assert(sw != nullptr);

  handleArrivals(sw);

  // reset matchings in the previous time slot
  std::fill(_in_match.begin(), _in_match.end(), -1);
  std::fill(_out_match.begin(), _out_match.end(), -1);

  // initialize vectors

  // records outport port which is being requested by the input port
  std::vector<int> requests(_num_inputs, -1);
  // records input port whose request is "accepted" by the output port
  std::vector<int> last_check(_num_outputs, -1);
  // list of input ports
  std::vector<int> inputs(_num_inputs, 0);
  // list of pairs of (input port, outport port) with root of BST
  std::vector<std::pair<std::pair<int, int>, int>> restore_sd_pairs;

  for (int it = 0; it < _iterations; ++it) {
    // reset requests & last_check
    std::fill(requests.begin(), requests.end(), -1);
    std::fill(last_check.begin(), last_check.end(), -1);

    // shuffle to select random input in Step 1
    for (int i = 0; i < _num_inputs; ++i)
      inputs[i] = i;
    std::shuffle(inputs.begin(), inputs.end(), _eng);

    // Step 1: Proposing
    for (int i = 0; i < _num_inputs; ++i) {
      int in = inputs[i];

      // no packets currently in queue or input already matched
      if (getQueueLength(in) == 0 || _in_match[in] != -1)
        continue; 

      // sample an output for this input and record it in requests
      auto out = sampling(in); 
      requests[in] = out;      

      // if output port has been matched already
      if (_out_match[out] != -1)
        continue;

      if (last_check[out] != -1) { // if requested before
        // assign appropriate input port according to QPS accept policy
        if (_accept_policy == "longest_first") {
          if (sw->get_queue_length(in, out) >
              sw->get_queue_length(last_check[out], out)) {
            last_check[out] = in;
          }
        } else if (_accept_policy == "earliest_first" ||
                   _accept_policy == "random") {
          // do nothing, previous input port is fine
        } else if (_accept_policy == "shortest_first") {
          if (sw->get_queue_length(in, out) <
              sw->get_queue_length(last_check[out], out)) {
            last_check[out] = in;
          }
        } else if (_accept_policy == "smallest_first") {
          if (in < last_check[out]) {
            last_check[out] = in;
          }
        }
      } else { // the first one
        last_check[out] = in;
      }
    }

#ifdef _DEBUG_QPS_
    std::cerr << "Iteration " << it << " :\n";
    std::cerr << "\trequests : " << requests << "\n";
    std::cerr << "\tlast_check : " << last_check << "\n";
#endif

    // Step 2: Accepting
    for (int out = 0; out < _num_outputs; ++out) {
      int in = last_check[out];

      // if in is accepted by out and in is unmatched
      if (in != -1 && _in_match[in] == -1) {
        _out_match[out] = in;
        _in_match[in] = out;
      }
    }

    // for without replacement, we need to change the queue length of certain
    // VOQ in the BST to 0
    if (_without_replacement && _iterations > 0) {
      for (int in = 0; in < _num_inputs; ++in) {
        if (_in_match[in] == -1) {
          int out = requests[in];
          if (out != -1) {
            auto old = removeVOQ(in, out);
            assert(old == sw->get_queue_length(in, out));
            restore_sd_pairs.push_back({{in, out}, old});
          }
        }
      }
    }
  }

  restoreVOQ(restore_sd_pairs);

  handleDepartures(sw);
}

void QPS::init(const IQSwitch *sw) {
  for (int s = 0; s < _num_inputs; ++s)
    for (int d = 0; d < _num_outputs; ++d)
      if (sw->get_queue_length(s, d) > 0)
        BST::update<int>(_bst[s], d + _left_start, sw->get_queue_length(s, d));
}
} // namespace saber
