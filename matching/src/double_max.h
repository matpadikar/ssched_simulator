// sb_qps.h header file
// Small-Batch (Maybe based on) Queue-Proportional Sampling
//

#ifndef DOUBLE_MAX_H
#define DOUBLE_MAX_H

#include "qp_sampler.h"
#include <fstream>
#include "sb_qps.h"

namespace saber {
//class SB_QPS_Fixed;

/** @brief Class for Half & Half SB_QPS
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-1 but allowing using holes left
 *  before to accept inputs' proposals.
 *
 */
class EdgeColorDoubleMax{
	protected:
		using bitmap_t = std::bitset<128>;
		static const int NULL_SCHEDULE;

		const size_t _frame_size;
		const int _num_inputs, _num_outputs;

		// virtual input queues
		std::vector<saber::Max_Propose> _input_queues;
		std::vector<int> _schedule;


		const bool _debug_mode{false};
		std::ofstream _debug_stream;

		// counter of packets
		std::vector<int> _output_availability;
		std::vector<int> _input_availability;
			
		int queue_length(int source) {
			assert(source >= 0 && source < _num_inputs);
			return _input_queues[source].get_queue_length();
		}
		// performs qps scheduling
		virtual void qps();
		void match_element(int in, int out);


			public:
		EdgeColorDoubleMax(const SB_QPS_Configuration& config);
				const int _cutoff;
				void bitmap_reset();
				void handle_arrivals(const std::vector<std::vector<int> >& weights);
				void handle_departures(const std::vector<std::pair<int, int>>& dep_pre);

		~EdgeColorDoubleMax() {
			if (_debug_mode){
				_debug_stream.close();
			}
		}
		// called by simulator to schedule the switches
		int schedule(const std::vector<std::vector<int>>& weights);
		//void init(const IQSwitch* sw) override;
		void reset() ;
		void display(std::ostream& os) const;
		//// reserved
		void dump_stats(std::ostream& os)  {}

		}; // SB_QPS_FIXED
} //namespace saber
#endif // SB_QPS_H
