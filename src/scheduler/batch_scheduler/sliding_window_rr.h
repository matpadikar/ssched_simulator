// Sliding Window (instead of batch) Queue-Proportional Sampling

#ifndef SLIDING_WINDOW_RR_H
#define SLIDING_WINDOW_RR_H

#include "batch_scheduler.h"
#include "switch/iq_switch.h"
#include "scheduler/rr_sampler.h"
#include <fstream>

namespace saber {

/** @brief QPS-1 for each time slot in sliding window and accept multiple proposals in round robin manner
 *
 *  For each time slot QPS-1 is run where an output port accepts multiple input port
 *  proposals. For each accepted input port, the output port tries to schedule it
 *  an empty/available time slot starting with the input port chosen in round robin manner 
 *  and from the current time slot wrapping to the start of the sliding window of fixed size. 
 *
 */
class Sliding_Window_RR :public BatchScheduler {
	/// Allow SchedulerFactory class to access protected variables and methods
	friend class SchedulerFactory;

	protected:
		/**
     * Protected constructor to be used by scheduler factory
     *
     * @param name name for an instance of the class
     * @param numInputs number of input ports in the switch
     * @param numOutputs number of output ports in the switch
     * @param batchSize number of time slots for which scheduling will be done together
     * @param seed  value of seed provided for random number generation
     * @param maxAccepts maximum number of input port proposals accepted by an output port
		 * @param batching set to true if batching is used instead of sliding window
     * @param debug_mode if true, more information is printed to terminal for debugging
     *
     */
		Sliding_Window_RR(std::string name, int numInputs,int numOutputs, int batchSize, std::mt19937::result_type seed, int maxAccepts, bool batching, bool debug_mode);
		
		using bitmap_t = std::bitset<128>;

		/// Value that represents empty schedule or not yet matched
		static const int NULL_SCHEDULE;

		/// Random number generation seed
		std::mt19937::result_type seed;
		/// Random number generation engine
		std::mt19937 eng{ std::random_device{}() };

		/// For a output port stores the largest input port it accepted
		std::vector<int> previousLargestInput;

		/// Queue of packets present at each input port from which output port will be sampled
		std::vector<saber::Round_Robin_Sampler> inputQueues;

		/// Bitmaps for input ports to represent which time slots are already matched
		std::vector<bitmap_t> inputMatchBitmap;
		/// Bitmaps for output ports to represent which time slots are already matched
		std::vector<bitmap_t> outputMatchBitmap;

		/// Boolean falg to decide if more information should be printed to terminal for debugging
		const bool _debug_mode{false};
		/// Stream to which the extra information is written to
		std::ofstream _debug_stream;

		/// Number of previous time slots in the batch when each output is available
		std::vector<int> outputAvailability;
		/// Number of previous time slots in the batch when each input is available
		std::vector<int> inputAvailability;

		/// Maximum number of input port proposals accepted by an output port
		const int maxAccepts;
		/// Set to true if scheduling done batch-by-batch like SB-QPS, rather than sliding window
		const bool batching;
		
		/**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
		void resetBitmap();

		/**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
		void handlePacketArrivals(const IQSwitch* iqSwitch);

		/**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
		void handlePacketDepartures(const std::vector<std::pair<int, int>>& virtualDepartures);

		/**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
		int getQueueLengthForInput(int input){
			assert(input >= 0 && input < _num_inputs);
			return inputQueues[input].get_queue_length();
		}

		/**
			* Since we are using sliding window, schedule for the time slot is reset as soon 
			* as it is copied to the simulator and input and output availability is updated
			*
			* @param timeSlot time slot for which schedule is to be dispatched and then reset
			*/
		void dispatchSchedule(int timeSlot);
		
		/**
      * Runs the logic for SW-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SW-QPS is run
      */
		virtual void qps(const IQSwitch* iqSwitch, size_t timeSlot);

	public:
		~Sliding_Window_RR() override {
			if (_debug_mode){
				_debug_stream.close();
			}
		}

		/**
      * Schedules the arrived packets for the current time slot
      */
		void schedule(const IQSwitch* iqSwitch) override;

		/**
      * Initializes variable values and/or performs any other initial tasks
      */
		void init(const IQSwitch* iqSwitch) override;

		/**
      * Resets all the variable values to initial values
      */
		void reset() override;

		/**
      * Prints out current values of the variables
      */
		void display(std::ostream& os) const override;
		
		/**
     * Reserved for future use
     */
		void dumpStats(std::ostream& os) {}

}; ///< Sliding_Window_RR

} ///< namespace saber

#endif ///< SLIDING_WINDOW_RR_H
