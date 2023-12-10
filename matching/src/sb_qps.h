// sb_qps.h header file
// Small-Batch (Maybe based on) Queue-Proportional Sampling
//

#ifndef SB_QPS_H
#define SB_QPS_H

#include "qp_sampler.h"

#include <fstream>
#include <string>
#include <bitset>
#include <cassert>
#include <condition_variable>
#include <iostream>
#include <thread>
#include "availmap.hpp"
#include <future>

namespace saber {
enum class SelectionMethod {
      		MAX = 0, MERGE = 1, FIRST = 2, MIXED = 3, TWOLAYER = 4
    	};
class SB_QPS_Configuration{
public:
	std::string name;
	std::vector<unsigned> seed_list;
	int num_inputs, cutoff, num_threads;
	int num_outputs, frame_size,  num_rounds, group_size, pool_size;
	SelectionMethod slm;
	bool centralize, vdepart, debug_mode, rqps_serena, use_availmap;
	std::mt19937::result_type seed;	
	std::string debug_stream;

	explicit SB_QPS_Configuration(std::ifstream& is){
		if (is.is_open()) {// stream is valid{
			is >> num_inputs >> num_outputs >> frame_size >> group_size >> num_rounds >> cutoff >> pool_size >> num_threads;
			int slm_int;
			is >> slm_int;
			slm = (SelectionMethod) slm_int;
			is >> centralize >> vdepart;
			is >> debug_mode >> rqps_serena >> use_availmap;
			is >> seed_list;
			is >> debug_stream;
		} else { // default invalid config
			num_inputs = 1;
			num_threads = 1;
			num_outputs = 1;
			frame_size = 1;
			num_rounds = 1;
			group_size = 1;
			pool_size = 1;
			cutoff = 1;
			slm = SelectionMethod::FIRST;
			centralize = false;
			vdepart = false;
			use_availmap = false;
			debug_mode = false;
			rqps_serena = false;
			seed = 0;
		}
		
	}
};

/** @brief Class for Half & Half SB_QPS
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-1 but allowing using holes left
 *  before to accept inputs' proposals.
 *
 */
class SB_QPS_Fixed{
	public:
		

	protected:
		using bitmap_t = std::bitset<1024>;
		static const int NULL_SCHEDULE;
		static constexpr int BIT_SIZE = 1024;

		const size_t _frame_size, _group_size;
		const int _num_inputs, _num_outputs;
		std::mt19937::result_type _seed;
		std::mt19937 _eng{ std::random_device{}() };

		// virtual input queues
		std::vector<QuantizedProposer> _input_queues;
		std::vector<std::vector<int>> _schedules;
		//std::vector<std::mutex> _mutexes;
		//std::vector<std::condition_variable> _condvs;

		// bitmaps for each input & output
		//std::vector<bitmap_t> _match_flag_in;
		//std::vector<bitmap_t> _match_flag_out;
		std::vector<AvailabilityMap<BIT_SIZE>> _match_avail_in;
		std::vector<AvailabilityMap<BIT_SIZE>> _match_avail_out;

		std::vector<std::vector<int>> out_accepts;

		//const bool _debug_mode{false};
		//std::ofstream _debug_stream;

		// counter of packets
		std::vector<int> _output_availability;
		std::vector<int> _input_availability;

		int _num_rounds, _weight;
		bool _use_availmap;
		SelectionMethod _selection;
		bool _centralized, _vdepart, _rqps_serena;
			
		//int queue_length(int source) {
		//	assert(source >= 0 && source < _num_inputs);
		//	return _input_queues[source].get_queue_length();
		//}
		void dispatch_schedule(int frame_id);
		// performs qps scheduling
		virtual void qps(int round, const std::vector<std::vector<int>>& weights);
		void match_element(int frame, int in, int out);
		

		// select first matching from schedule
		std::vector<int> select_first(const std::vector<std::vector<int>>& weights);
		std::vector<int> select_heaviest(const std::vector<std::vector<int>>& weights);
		std::vector<int> serena_merge(const std::vector<std::vector<int>>& weights);
		std::vector<int> mixed_max_merge(const std::vector<std::vector<int>>& weights);
		std::vector<int> mixed_merge_merge(const std::vector<std::vector<int>>& weights);


		//void matching_thread(int id, int num_threads, std::vector<std::mutex>& mutexes, 
		//	std::vector<std::condition_variable>& condvs, const std::vector<std::vector<int>>& weights,
		//	std::vector<int>& signals);


			public:
		SB_QPS_Fixed(const SB_QPS_Configuration& config);

		const int _cutoff;
		void bitmap_reset();
		void handle_arrivals(const std::vector<std::vector<int> >& weights);
		void handle_departures(const std::vector<std::pair<int, int>>& dep_pre);

		~SB_QPS_Fixed() {
			//if (_debug_mode){
			//	_debug_stream.close();
			//}
		}
		static int weight_computation(
			const std::vector<int>& matching, const std::vector<std::vector<int>>& weights);
		// called by simulator to schedule the switches
		std::vector<int> schedule(const std::vector<std::vector<int>>& weights);
		std::vector<int> schedule(const std::vector<std::vector<int>>& weights, 
		double& time_qps, double& time_serena);
		//void init(const IQSwitch* sw) override;
		void reset() ;
		void display(std::ostream& os) const;
		//// reserved
		void dump_stats(std::ostream& os)  {}

		int matching_weight() const{return _weight;}
		}; // SB_QPS_FIXED

	class ParallelScheduler{
		private:
		int _num_threads;  // initialize this first

		std::vector<std::thread> _threads;
		//std::vector<std::condition_variable> _condvs;
		//std::vector<std::mutex> _mutexes;
		//std::vector<SB_QPS_Fixed> _qps;
		std::vector<std::future<std::vector<int>>> _futures;
		SB_QPS_Configuration _config;
		

		//std::vector<std::vector<int> > _schedules;
		//std::vector<int> _signals;

		void join_all() {
			for(auto& tr : _threads){
				tr.join();
			}
		}

		void matching_thread(int id, const std::vector<std::vector<int>>& weights, 
										std::promise<std::vector<int>>&& prom);

		public:
		ParallelScheduler (SB_QPS_Configuration config);
		std::vector<int> compute(const std::vector<std::vector<int>>& weights);

	};
} //namespace saber
#endif // SB_QPS_H
