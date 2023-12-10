
#include "sb_qps.h"
#include <functional>
#include <queue> // std::priority_queue
#include <algorithm>
#include "serena/serena.hpp"
#include "timer.hpp"


namespace saber {
const int SB_QPS_Fixed::NULL_SCHEDULE = -1;
// ////////////////////////////////////////////////////////////////////////////
//        Implementation of class SB_QPS_Fixed
SB_QPS_Fixed::SB_QPS_Fixed( const SB_QPS_Configuration& config)
	: _weight(NULL_SCHEDULE), _num_inputs(config.num_inputs), _num_outputs(config.num_outputs),
	_frame_size(config.frame_size), 
	_seed(config.seed), _eng(config.seed), //_match_flag_in(config.num_inputs),
	_centralized(config.centralize), _cutoff(config.cutoff), _group_size(config.group_size),
	//_match_flag_out(config.num_outputs), //_debug_mode(config.debug_mode), 
  	_num_rounds(config.num_rounds), _selection(config.slm), _vdepart(config.vdepart),
	_output_availability(config.num_outputs, config.frame_size), _rqps_serena(config.rqps_serena),
	_schedules(_frame_size, std::vector<int>(_num_inputs, NULL_SCHEDULE)),
	_use_availmap(config.use_availmap), _match_avail_in(config.num_inputs, AvailabilityMap<BIT_SIZE>(config.frame_size)), 
	_match_avail_out(config.num_outputs, AvailabilityMap<BIT_SIZE>(config.frame_size)),
	out_accepts(_num_outputs), 
	_input_availability(config.num_inputs, config.frame_size) {
	_input_queues.reserve(config.num_inputs);
	for (size_t i = 0; i < config.num_inputs; ++i)
		_input_queues.emplace_back(config.pool_size, _num_outputs);
	//_cf_rel_time = 0;
	// generate initial schedulers (used for the first frame)
	for (auto& sched : _schedules) {
		std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	}
	for (auto& oa : out_accepts){
		oa.reserve(_cutoff);
	}
	//for (auto& sched : _schedules_pre) {
	//	std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	//}

	//if(_debug_mode){
	 //   _debug_stream.open("debug_dump.txt", std::ofstream::out);
	 //   _debug_stream << "scheduling starts\n";
	//}
	//else{
    //    _debug_stream.setstate(std::ofstream::badbit);
        // disable _debug_stream
	//}

	if (_rqps_serena){
		// lock number of rounds in rpqs serena mode
		_num_rounds = _frame_size;
	}
}

void SB_QPS_Fixed::bitmap_reset() {
	/*for (auto& mf : _match_flag_in)
		mf.reset();
	for (auto& mf : _match_flag_out)
		mf.reset();*/
	for (auto& availm: _match_avail_in){
		availm.reset();
	}
	for (auto& availm: _match_avail_out){
		availm.reset();
	}
	//std::fill(_input_availability.begin(), _input_availability.end(), _frame_size);
	//std::fill(_output_availability.begin(), _output_availability.end(), _frame_size);
}

void SB_QPS_Fixed::handle_arrivals(const std::vector<std::vector<int>>& weights) {
	reset();
	for (int i = 0; i < weights.size(); ++i) {
		_input_queues[i] = weights[i];
	}                                                                                                                                                                                                                                                                                                    
	//_debug_stream << "\n";
}
/*
void SB_QPS_Fixed::handle_departures(
	const std::vector< std::pair<int, int>>& dep_pre) {
	for (const auto& sd : dep_pre) {
		assert(_input_queues[sd.first].get_packet_number(sd.second) > 0);
		_input_queues[sd.first].remove(sd.second);
	}
}*/

void SB_QPS_Fixed::qps(int round, const std::vector<std::vector<int>>& weights) {
/*
    _debug_stream << "queue state\n" << std::setw(11) << " ";
    for (int f=0; f<_num_inputs; ++f)
    {
        _debug_stream << std::setw(3) << f;
    }
    _debug_stream << "\n";
    for (size_t in=0; in<_num_inputs; ++in){
        _debug_stream << "input " << std::setw(3) << in << ": " << _input_queues[in] << "\n";
    }*/

    // proposals recieved by each output port
	
    //std::vector<std::vector<int>> out_accepts(_num_outputs);
	StopWatch sw(false);
	for (auto& oa: out_accepts) {
		oa.clear();
	}

	/*//std::vector<int> inputs(_num_inputs, 0);
	for (int i = 0; i < _num_inputs; ++i)
		inputs[i] = i;
    // shuffle inputs to simulate the random execution sequence of different processors
	if (!_centralized){	
		std::shuffle(inputs.begin(), inputs.end(), _eng);
	}*/
	//std::cout << sw.peek() << "\t";
	sw.start();
    // Step 1: Proposing
    for (int i = 0; i < _num_inputs; ++i) {
        int in = i;
        if (!_input_queues[in].is_good())
                continue;              // no packets
        int out =  _input_queues[in].sample(_eng); // sampling an output for this input
		//int out = in;
        //assert(_input_queues[in].get_packet_number(out) > 0);
		if (out_accepts[out].size()<_cutoff){ // only accepts up to #cutoff proposals
			out_accepts[out].push_back(in); // this is slow, so make out_acceps a member variable
		}	
    }
	//std::cout << sw.peek() << std::endl;
	sw.reset_and_start();
    //std::vector<std::pair<int, int>> vdep;  virtual departures
    // matching for output ports
    //std::vector<int> out_match(num_outputs(), NULL_SCHEDULE);
    // Step 2: Accept
    //_debug_stream << "accepting\n";
    for (int out = 0; out < _num_outputs; ++out) {
        if (out_accepts[out].empty())  // no proposal received
                continue;

        // sort proposals based on VOQ lengths
        // actually we can optimize by move sorting inside the if check
        // and whenever if fails, we just find the largest element and
        // move it to the beginning of the vector!
		// capture _input_queues by reference!!
        //std::sort(out_accepts[out].begin(), out_accepts[out].end(),
         //   [&](const int in1, const int in2) {
         //           //return _input_queues[in1].get_packet_number(out) >
                     //       _input_queues[in2].get_packet_number(out);
					// return weights[in1][out] > weights[in2][out];
         //  });

        for (int in: out_accepts[out]) {
            if (_output_availability[out] == 0)
            {/*
                if(_debug_mode){
                    _debug_stream << "(" <<  in << ", " << out << ", -)" ;
                    continue;
                } else {
                    break;
                }*/
				break;
            }
            //_debug_stream << "(" <<  in << ", " << out << ", ";
            if (_input_availability[in] == 0)
                    continue; // no available ts
                    // available only when both available
			if (_rqps_serena){
				if (round < _frame_size) match_element(round, in, out);
				break;
			} else {
				if (_use_availmap){
					auto amap = _match_avail_in[in] & _match_avail_out[out];
					int f = amap.ffs();

					//std::cout << f << std::endl;
					if (f >= 0 && f < _frame_size){
						match_element(f, in, out);
					}
					//if (f == _frame_size-1) break;
				} /*else {
					auto mf = (_match_flag_in[in] | _match_flag_out[out]);
					//bool success = false;
					for (int f=0; f<(int)_frame_size; ++f) {
						if (!mf.test(f)) {
							match_element(f, in, out);
							//vdep.emplace_back(in, out);
							//_debug_stream << f << ")";
							//success = true;
							break; // pay special attention (first fit)
						}
					}
				}*/
			}
        }
		
    }
	//std::cout << sw.peek() << std::endl;
    //_debug_stream << "\n";

#ifdef DEBUG
	std::cerr << _cf_packets_counter << "\n";
	std::cerr << vdep << "\n\n";
#endif
	//handle_departures(vdep);
}

void SB_QPS_Fixed::match_element(int frame, int in, int out){
	if (_use_availmap){
		_match_avail_in[in].set(frame);
		_match_avail_out[out].set(frame);
		//_match_flag_in[in].set(frame);
		//_match_flag_out[out].set(frame);
	} /*else {
		_match_flag_in[in].set(frame);
		_match_flag_out[out].set(frame);
	}*/

	--_input_availability[in];
	--_output_availability[out];
	assert(_schedules[frame][in] == NULL_SCHEDULE);
	_schedules[frame][in] = out;
	//if (_vdepart)
	//	_input_queues[in].remove(out);
}

//void SB_QPS_Fixed::init(const IQSwitch* sw) {
	// reserved
//}

std::vector<int> SB_QPS_Fixed::schedule(const std::vector<std::vector<int>>& weights) {
	assert(weights.size() == _num_inputs);
	StopWatch tt;
	for(int i = 0; i < _num_inputs; ++i){
		assert(weights[i].size() == _num_outputs);
	}

	handle_arrivals(weights);
    // dispatch schedule
    //
	std::cout << "arrival:\t" << tt.peek() << std::endl;
	for (int round = 0; round < _num_rounds; ++round){
		qps(round, weights);
	}
   //std::cout << "Qps:\t" << tt.peek() << std::endl;
    /*
	_debug_stream << "buffer state\n" <<std::setw(11) << " ";
	for (int f=0; f<_frame_size; ++f)
    {
	    _debug_stream << std::setw(3) << f;
    }
	_debug_stream << "\n";
	for (size_t f=0; f<_frame_size; ++f){
	    _debug_stream << "frame " << std::setw(3) << f << ": ";
	    for (int out_port:_schedules[f]){
            _debug_stream << std::setw(3) << out_port;
	    }
	    _debug_stream << "\n";
	}
	
	if (frame_id == _frame_size-1){ //this batch ends
		//schedules_pre should be empty now
		if (_debug_mode)
		{
			for (auto schedule:_schedules_pre)
			{
				for (auto cell: schedule)
				{
					assert(cell == NULL_SCHEDULE);
				}
			}
		}
		
		// reset availability for _schedule
		bitmap_reset();
	}*/

	for (int i = 0; i < _frame_size; ++i){
		fix_matching(_schedules[i]);
	}

	std::vector<int> result;
	if (_selection == SelectionMethod::FIRST){
		result = select_first(weights);
	} else if (_selection == SelectionMethod::MAX){
		result = select_heaviest(weights);
	} else if (_selection == SelectionMethod::MERGE){
		result = serena_merge(weights);
	} else if (_selection == SelectionMethod::MIXED){
		result = mixed_max_merge(weights);
	} else if (_selection == SelectionMethod::TWOLAYER){
		result = mixed_merge_merge(weights);
	}
	//std::cout << "merge:\t" << tt.peek() << std::endl << std::endl;
	return result;
}


std::vector<int> SB_QPS_Fixed::schedule(const std::vector<std::vector<int>>& weights, 
		double& time_qps, double& time_serena) {
	assert(weights.size() == _num_inputs);
	for(int i = 0; i < _num_inputs; ++i){
		assert(weights[i].size() == _num_outputs);
	}

	StopWatch watch;
	handle_arrivals(weights);
	//std::cout << "arrival\t" << watch.peek() << "\t";
    // dispatch schedule
	for (int round = 0; round < _num_rounds; ++round){
		qps(round, weights);
	}
	time_qps = watch.peek();

	//std::cout << "QPS " << time_qps << std::endl;

	watch.reset_and_start();
	for (int i = 0; i < _frame_size; ++i){
		fix_matching(_schedules[i]);
	}

	std::vector<int> result;
	if (_selection == SelectionMethod::FIRST){
		result = select_first(weights);
	} else if (_selection == SelectionMethod::MAX){
		result = select_heaviest(weights);
	} else if (_selection == SelectionMethod::MERGE){
		result = serena_merge(weights);
	} else if (_selection == SelectionMethod::MIXED){
		result = mixed_max_merge(weights);
	} else if (_selection == SelectionMethod::TWOLAYER){
		result = mixed_merge_merge(weights);
	}

	time_serena = watch.peek();
	return result;
}

std::vector<int> SB_QPS_Fixed::select_first(const std::vector<std::vector<int>>& weights){
	_weight = weight_computation(_schedules[0], weights);
	return _schedules[0];
}

std::vector<int> SB_QPS_Fixed::select_heaviest(const std::vector<std::vector<int>>& weights){
	std::vector<int> matching_weights(_frame_size);
	for(int i = 0; i < _frame_size; ++i){
		matching_weights[i] = weight_computation(_schedules[i], weights);
	}
	auto iter = std::max_element(matching_weights.begin(), matching_weights.end());
	int heaviest_frame = iter - matching_weights.begin();
	_weight = matching_weights[heaviest_frame];
	return _schedules[heaviest_frame];
}

std::vector<int> SB_QPS_Fixed::serena_merge(const std::vector<std::vector<int>>& weights){
	std::vector<int> merged_matching = _schedules[0];
	for(int i = 1; i < _frame_size; ++i){
		merge(merged_matching, _schedules[i], merged_matching, weights);
	};
	_weight = weight_computation(merged_matching, weights);
	return merged_matching;
}



std::vector<int> SB_QPS_Fixed::mixed_max_merge(const std::vector<std::vector<int>>& weights){
	int group_num = _frame_size / _group_size + ((_frame_size % _group_size)? 1: 0);
	int max_weight = -1;
	std::vector<int> max_matching;
	
	for(int g = 0; g < group_num; ++g){
		int i = g * _group_size;
		std::vector<int> merged_matching = _schedules[i];
		for(++i; i < _frame_size && i < (g+1) * _group_size; ++i){
			merge(merged_matching, _schedules[i], merged_matching, weights);
		};
		int weight = weight_computation(merged_matching, weights);
		//std::cout << weight << "\t" << max_weight << std::endl;
		if (weight > max_weight){
			max_matching = std::move(merged_matching);
			max_weight = weight;
		}
	}
	_weight = max_weight;
	return max_matching;
}

std::vector<int> SB_QPS_Fixed::mixed_merge_merge(const std::vector<std::vector<int>>& weights){
	int group_num = _frame_size / _group_size + ((_frame_size % _group_size)? 1: 0);
	std::vector<int> result_matching;
	
	for(int g = 0; g < group_num; ++g){
		int i = g * _group_size;
		std::vector<int> merged_matching = _schedules[i];
		for(++i; i < _frame_size && i < (g+1) * _group_size; ++i){
			merge(merged_matching, _schedules[i], merged_matching, weights);
		};
		if (g == 0){
			result_matching = std::move(merged_matching);
		} else {
			merge(result_matching, merged_matching, result_matching, weights);
		}
	}
	_weight = weight_computation(result_matching, weights);
	return result_matching;
}

int SB_QPS_Fixed::weight_computation(
	const std::vector<int>& matching, const std::vector<std::vector<int>>& weights){
	int result = 0;
	for(int i = 0; i < matching.size(); ++i){
		if (matching[i] != NULL_SCHEDULE){
			result += weights[i][matching[i]];
		}
	}
	return result;
}

void SB_QPS_Fixed::reset() {
	//BatchScheduler::reset();
	bitmap_reset();
	std::fill(_input_availability.begin(), _input_availability.end(), _frame_size);
	std::fill(_output_availability.begin(), _output_availability.end(), _frame_size);
	for (size_t i = 0; i < _num_inputs; ++i)
		_input_queues[i].reset();
	//_cf_rel_time = 0;
	for (auto& sched : _schedules) {
              //    for (size_t in = 0; in < num_inputs(); ++in) sched[in] = in;
              //    std::shuffle(sched.begin(), sched.end(), _eng);
        std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	}
	//for (auto& sched : _schedules_pre) {
      //        std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	//}
}

void SB_QPS_Fixed::display(std::ostream& os) const {
	//BatchScheduler::display(os);
	os << "---------------------------------------------------------------------"
		"\n";
	os << "seed             : " << _seed << "\n input queues\n";
	for (const auto & item : _input_queues) {
		//os << item << "\n";
	}
		
}


ParallelScheduler::ParallelScheduler (SB_QPS_Configuration config):
_num_threads(config.num_threads), 
_threads(_num_threads),  _config(config)
{
	/*for(int t = 0; t < _num_threads; ++t){
		config.seed = config.seed_list[t];
		_qps.emplace_back(config);_mutexes(_num_threads), _condvs(_num_threads),_schedules(_num_threads),
	}*/
}

std::vector<int> ParallelScheduler::compute(const std::vector<std::vector<int>>& weights){
	//_signals.assign(_num_threads, 0);
	StopWatch tt(false);
	_futures.clear();
	std::vector<std::promise<std::vector<int>>> promises(_num_threads);

	for(int id = 0; id < _num_threads; ++id){
		_futures.push_back(promises[id].get_future());
	}

	for(int id=0; id < _num_threads; ++id){
		_threads[id] = std::thread(&ParallelScheduler::matching_thread, this, id, std::ref(weights), 
								   std::move(promises[id]));
	}
	//std::cout << "dispatch\t" << tt.peek() << std::endl;
	//tt.reset_and_start();
	tt.start();
	for(auto& tr : _threads){
		tr.join();
	}
	//std::cout << "join\t" << tt.peek() << std::endl;
	return _futures[0].get();
}

void ParallelScheduler::matching_thread(int id, const std::vector<std::vector<int>>& weights, 
										std::promise<std::vector<int>>&& prom){
	StopWatch tt;
	SB_QPS_Configuration config = _config;
	config.seed = config.seed_list[id];
	
	SB_QPS_Fixed qps(config);
	
	std::vector<int> schedule, received;
	/*{
		std::lock_guard<std::mutex> lck(_mutexes[id]);
		_schedules[id] = qps.schedule(weights);
		//std::cout << id << "\t" << weight_computation(_schedules[my_work_start], weights) << std::endl;
	}
	_condvs[id].notify_one();*/
	schedule = qps.schedule(weights);
	//if (id == 0) std::cout << "QPS Schedule\t" << tt.peek() << std::endl;
	
	int jump;
	for (int round = 1; jump = 1u << (round - 1), jump < _num_threads; ++round) {
		int mod = jump * 2;
		if (id % mod == 0 && id + jump < _num_threads){
			/*{
				std::unique_lock<std::mutex> lck(_mutexes[id + jump]);
				_condvs[id + jump].wait(lck, [this, id, jump, round]{ return _signals[id + jump] == round;});
			}  // wait for supplier.*/
			//tt.reset_and_start();
			received = _futures[id + jump].get();
			if (id == 0)
			//std::cout << "waiting:\t" << tt.peek() << std::endl;
			/*{
				std::lock_guard<std::mutex> lck(_mutexes[id]);
				merge(_schedules[id], _schedules[id + jump], _schedules[id], weights);
				++_signals[id]; // my work finishes
				//std::cout << round << "\t" << id << "\t" << weight_computation(_schedules[my_work_start], weights) << std::endl;
			}*/
			//_condvs[id].notify_one();
			merge(schedule, received, schedule, weights);
		}
	}

	prom.set_value(schedule);
	//if (id == 0) std::cout << "setting prom:\t" << tt.peek() << std::endl;
}

} // namespace saber
