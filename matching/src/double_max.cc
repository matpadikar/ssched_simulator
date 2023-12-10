
#include "double_max.h"
#include <functional>
#include <queue> // std::priority_queue
#include <algorithm>
#include "serena/serena.hpp"
#include "utilities.h"

namespace saber {
const int EdgeColorDoubleMax::NULL_SCHEDULE = -1;
// ////////////////////////////////////////////////////////////////////////////
//        Implementation of class SB_QPS_Fixed
EdgeColorDoubleMax::EdgeColorDoubleMax( const SB_QPS_Configuration& config)
	:  _num_inputs(config.num_inputs), _num_outputs(config.num_outputs),
	_frame_size(config.frame_size),
	_cutoff(config.cutoff),_debug_mode(config.debug_mode), 
	_output_availability(config.num_outputs, 1), 
	_schedule(config.num_inputs, NULL_SCHEDULE),
	_input_availability(config.num_inputs, 1) {
	_input_queues.reserve(config.num_inputs);
	for (size_t i = 0; i < config.num_inputs; ++i)
		_input_queues.emplace_back(_num_outputs);
	//_cf_rel_time = 0;
	// generate initial schedulers (used for the first frame)
	//for (auto& sched : _schedules_pre) {
	//	std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	//}

	if(_debug_mode){
	    _debug_stream.open("debug_dump.txt", std::ofstream::out);
	    _debug_stream << "scheduling starts\n";
	}
	else{
        _debug_stream.setstate(std::ofstream::badbit);
        // disable _debug_stream
	}
}


void EdgeColorDoubleMax::handle_arrivals(const std::vector<std::vector<int>>& weights) {
	reset();
	for (int i = 0; i < weights.size(); ++i) {
		_input_queues[i] = weights[i];
	}                                                                                                                                                                                                                                                                                                    
	_debug_stream << "\n";
}
/*
void SB_QPS_Fixed::handle_departures(
	const std::vector< std::pair<int, int>>& dep_pre) {
	for (const auto& sd : dep_pre) {
		assert(_input_queues[sd.first].get_packet_number(sd.second) > 0);
		_input_queues[sd.first].remove(sd.second);
	}
}*/

void EdgeColorDoubleMax::qps() {
    std::vector<std::vector<int>> out_accepts(_num_outputs);

    // Step 1: Proposing
    for (int in = 0; in < _num_inputs; ++in) {
        if (queue_length(in) == 0)
                continue;              // no packets
        int out = _input_queues[in].sample(); // sampling an output for this input
        assert(_input_queues[in].get_packet_number(out) > 0);
	
		if (out_accepts[out].size()<_cutoff){ // only accepts up to #cutoff proposals
			out_accepts[out].push_back(in);
		}
    }
    // Step 2: Accept
    _debug_stream << "accepting\n";
    for (int out = 0; out < _num_outputs; ++out) {
        if (out_accepts[out].empty())  // no proposal received
                continue;

        // sort proposals based on VOQ lengths
        // actually we can optimize by move sorting inside the if check
        // and whenever if fails, we just find the largest element and
        // move it to the beginning of the vector!
		// capture _input_queues by reference!!
        std::sort(out_accepts[out].begin(), out_accepts[out].end(),
            [&](const int in1, const int in2) {
				int diff = _input_queues[in1].get_packet_number(out) -
                            _input_queues[in2].get_packet_number(out);
				if (diff != 0) return diff > 0;
				else return in1 > in2;
            });

        for (int in: out_accepts[out]) {
            if (_output_availability[out] == 0)
            {
				break;
            }
            //_debug_stream << "(" <<  in << ", " << out << ", ";
            if (_input_availability[in] == 0)
                    continue; 
            match_element(in, out);
            
        }
    }
    //_debug_stream << "\n";

#ifdef DEBUG
	std::cerr << _cf_packets_counter << "\n";
	std::cerr << vdep << "\n\n";
#endif
	//handle_departures(vdep);
}

void EdgeColorDoubleMax::match_element(int in, int out){
	--_input_availability[in];
	--_output_availability[out];
	_schedule[in] = out;

	_input_queues[in].increment(out, -1);
}

//void SB_QPS_Fixed::init(const IQSwitch* sw) {
	// reserved
//}

int EdgeColorDoubleMax::schedule(const std::vector<std::vector<int>>& weights) {
	assert(weights.size() == _num_inputs);
	for(int i = 0; i < _num_inputs; ++i){
		assert(weights[i].size() == _num_outputs);
	}

	int round = 0, packets = matrix_sum(weights);
	handle_arrivals(weights);

	while(packets > 0){
		qps();
		int schedule_size = matching_size(_schedule);
		assert(schedule_size > 0);
		packets -= schedule_size;
		++round;

		if (_debug_mode){
			_debug_stream << round << "\t" << schedule_size << "\t" << _schedule << std::endl;
		}
		reset();
	}
	for(auto iq : _input_queues){
		assert(iq.is_empty());
	}
	
	return round;
}

void EdgeColorDoubleMax::reset() {
	//BatchScheduler::reset();
	std::fill(_input_availability.begin(), _input_availability.end(), 1);
	std::fill(_output_availability.begin(), _output_availability.end(), 1);
	for (size_t i = 0; i < _num_inputs; ++i)
		//_input_queues[i].reset();
              //    for (size_t in = 0; in < num_inputs(); ++in) sched[in] = in;
              //    std::shuffle(sched.begin(), sched.end(), _eng);
        std::fill(_schedule.begin(), _schedule.end(), NULL_SCHEDULE);
	//for (auto& sched : _schedules_pre) {
      //        std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	//}
}

void EdgeColorDoubleMax::display(std::ostream& os) const {
	//BatchScheduler::display(os);
	os << "---------------------------------------------------------------------"
		"\n";
	for (const auto & item : _input_queues) {
		os << item << "\n";
	}
		
}

} // namespace saber
