/// Small-Batch Queue-Proportional Sampling

#ifndef SB_QPS_H
#define SB_QPS_H

#include "batch_scheduler.h"
#include "scheduler/qp_sampler.h"
#include "scheduler/priority_queue.h"
#include <fstream>

namespace saber {

/** @brief QPS-1 in every time slot
 *
 *  Basic case where just QPS-1 is run in each time slot of the batch
 *  without any batch optimizations.
 *
 */
class SB_QPS_Basic : public BatchScheduler {
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
     *
     */
    SB_QPS_Basic(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};
    
    using bstT = std::vector<int>;
    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    std::vector<bstT> bstForSamplingAtInput;
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};
    
    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;

    using bitmapT = std::bitset<128>;
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT> ouputMatchBitmap;

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap();

  public:
    ~SB_QPS_Basic() override = default;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch);

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;
    
    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;
    
    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_Basic



/** @brief First half run QPS-1 and second half run QPS-1 with one more accept
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-1 but additionally filling one
 *  extra earlier available time slot.
 *
 */
class SB_QPS_HalfHalf_Oblivious : public BatchScheduler {
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
     *
     */
    SB_QPS_HalfHalf_Oblivious(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    using bstT = std::vector<int>;
    std::vector<bstT> bstForSamplingAtInput;
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;

    using bitmapT = std::bitset<128>;
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT> ouputMatchBitmap;

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap();

  public:
    ~SB_QPS_HalfHalf_Oblivious() override = default;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_HalfHalf_Oblivious



/** @brief First half run QPS-1 and second half run QPS-1 with as many possible more accepts
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-1 but additionally filling 
 *  as many as possible extra earlier available time slots.
 *
 */
class SB_QPS_HalfHalf_AvailabilityAware : public BatchScheduler {
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
     *
     */
    SB_QPS_HalfHalf_AvailabilityAware(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    using bstT = std::vector<int>;
    std::vector<bstT> bstForSamplingAtInput;
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;

    using bitmapT = std::bitset<128>;
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT>  inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT>  ouputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap();

  public:
    ~SB_QPS_HalfHalf_AvailabilityAware() override = default;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_HalfHalf_AvailabilityAware


/** @brief Run QPS-1 with an adaptive number of more accepts
 *
 *  For each time slot doing QPS-1 but additionally filling 
 *  an adaptive number of extra earlier available time slots.
 *
 */
class SB_QPS_Adaptive : public BatchScheduler {
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
     *
     */
    SB_QPS_Adaptive(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);

    using bstT = std::vector<int>;
    using bitmapT = std::bitset<128>;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    std::vector<bstT> bstForSamplingAtInput;

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;
    
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT> ouputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap() ;

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

  public:
    ~SB_QPS_Adaptive() override = default;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_Adaptive



/** @brief First half run QPS-1 and second half run QPS-3 with one more accept
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-3 but additionally filling 
 *  one extra earlier available time slot.
 *
 */
class SB_QPS_HalfHalf_MI : public BatchScheduler {

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
     *
     */
    SB_QPS_HalfHalf_MI(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);
    using bstT = std::vector<int>;
    using bitmapT = std::bitset<128>;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    std::vector<bstT> bstForSamplingAtInput;

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;
    
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT>  inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT>  ouputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap() ;

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

  public:
    ~SB_QPS_HalfHalf_MI() override = default;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Overloaded function for running each iteration of the QPS 
      * when more than one iterations are to be run in each round
      *
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(size_t timeSlot);

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_HalfHalf_MI


/** @briefFirst half run QPS-1 and last one third run QPS-3 with three more accepts
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the last one third of the batch, doing QPS-3 but 
 *  additionally filling three extra earlier available time slots.
 *  Also for first one third of second half of the batch, doing QPS-2
 *  and additionally filling one extra earlier available time slot.
 *
 */
class SB_QPS_ThreeThird_MI : public BatchScheduler {

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
     *
     */
    SB_QPS_ThreeThird_MI(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);
    using bstT = std::vector<int>;
    using bitmapT = std::bitset<128>;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    std::vector<bstT> bstForSamplingAtInput;

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;
    
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT> ouputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap();

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

  public:
    ~SB_QPS_ThreeThird_MI() override = default;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Overloaded function for running each iteration of the QPS 
      * when more than one iterations are to be run in each round
      *
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(size_t timeSlot);

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_ThreeThird_MI


/** @brief First half run QPS-1 and second half run QPS-1 with a fixed number of more accepts
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-1 but additionally filling 
 *  a fixed number of extra earlier available time slots.
 *
 */
class SB_QPS_HalfHalf_MA : public BatchScheduler {

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
      *
      */
    SB_QPS_HalfHalf_MA(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);

    using bstT = std::vector<int>;
    using bitmapT = std::bitset<128>;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    std::vector<bstT> bstForSamplingAtInput;

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;
    
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT> ouputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap() ;

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

  public:
    ~SB_QPS_HalfHalf_MA() override = default;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Overloaded function for running each iteration of the QPS 
      * when more than one iterations are to be run in each round
      *
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(size_t timeSlot);

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_HalfHalf_MA


/** @brief First half run QPS-1 and second half run QPS-3 with a fixed number of more accepts
 *
 *  In the first half of a batch, just doing QPS-1 for each time slot,
 *  whereas in the second half, doing QPS-3 but additionally filling 
 *  a fixed number of extra earlier available time slots.
 *
 */
class SB_QPS_HalfHalf_MA_MI : public BatchScheduler {

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
     *
     */
    SB_QPS_HalfHalf_MA_MI(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed);

    using bstT = std::vector<int>;
    using bitmapT = std::bitset<128>;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};
    /// Index value needed for above implementation of the auxillary data structure
    int bstLeftStart{-1};

    /// Auxillary data structure for performing Queue-Proportional sampling of output port
    std::vector<bstT> bstForSamplingAtInput;

    /// Length of each Virtual Output Queue (count of packets in the particular VOQ)
    std::vector<std::vector<int>> VOQLength;
    
    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmapT> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmapT> ouputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap();

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

    /**
      * Samples an output port for a given input port using Queue-Proportional sampling
      *
      * @param input the input port for which output port is to be sampled
      * @returns the sampled output port
      */
    int sampleOutputForInput(int input);

    /**
      * Provides length of the entire packet queue at an input port
      *
      * @param input the input port for which queue length is needed
      * @returns total number packets in all the VOQs of the input port
      */
    int getQueueLengthForInput(int input){
      assert (input >= 0 && input < _num_inputs);
      return bstForSamplingAtInput[input][1];
    }

    /**
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(const IQSwitch *iqSwitch, size_t timeSlot);

  public:
    ~SB_QPS_HalfHalf_MA_MI() override = default;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) ;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Overloaded function for running each iteration of the QPS 
      * when more than one iterations are to be run in each round
      *
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    void qps(size_t timeSlot);

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override ;

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_HalfHalf_MA_MI

/** @brief Just QPS-1 for all time slots and batch optimization for last time slot
 *
 *  First we just do QPS-1 for each time slot but save the second largest proposals,
 *  which are then used in the last time slot to fill up earlier time slots where 
 *  output ports are not matched. Finally this entire process is repeated for a 
 *  fixed number of iterations.
 *
 */
class SB_QPS_Conditional : public BatchScheduler {

  /// Allow SchedulerFactory class to access protected variables and methods
  friend class SchedulerFactory;

  protected:
    /**
     * Protected constructor to be used by scheduler factory
     *
     * @param name name for an instance of the class
     * @param numInputs number of input ports in the switch
     * @param numOutputs number of output ports in the switch
     * @param numIterations number fo iterations for which to repeat the scheduling strategy
     * @param batchSize number of time slots for which scheduling will be done together
     * @param seed  value of seed provided for random number generation
     *
     */
    SB_QPS_Conditional(std::string name, int numInputs, int numOutputs, int numIterations, int batchSize, std::mt19937::result_type seed);
    
    using bitmap_t = std::bitset<128>;

    /// Value that represents empty schedule or not yet matched
    static const int NULL_SCHEDULE;

    /// Number fo iterations for which the scheduling strategy is repeated
    const int numIterations;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{std::random_device{}()};

    /// Queue of packets present at each input port from which output port will be sampled
    std::vector<saber::Queue_Sampler> inputQueues;

    /// Bitmaps for input ports to represent which time slots are already matched
    std::vector<bitmap_t> inputMatchBitmap;
    /// Bitmaps for output ports to represent which time slots are already matched
    std::vector<bitmap_t> outputMatchBitmap;

    /// Number of previous time slots in the batch when each output is available
    std::vector<int> outputAvailability;
    /// Number of previous time slots in the batch when each input is available
    std::vector<int> inputAvailability;

    /// Saves the second largest input ports that proposed to output ports in each time slot
    std::vector< std::vector<int> > conditionalApprovals;

    /**
      * Resets the inputMatchBitmap and outputMatchBitmap values
      */
    void resetBitmap();

    /**
      * Updates VOQs and auxillary data strcture using packet arrival information
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      */
    void handlePacketArrivals(const IQSwitch *iqSwitch);

    /**
      * Updates VOQs and auxillary data strcture using packet departure information
      * @param virtualDepartures input and output port information for departuring packets
      */
    void handlePacketDepartures(const std::vector<std::pair<int, int>> &virtualDepartures);

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
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      * @param iteration the current iteration number
      */
    void qps(size_t timeSlot, int iteration);

    /**
      * Updates the bitmaps, availability values once an input port and output port are
      * matched. Also removes the corresponding packet from the input ports queue.
      *
      * @param input input port that is matched
      * @param output output port that is matched
      * @param timeSlot time slot in the batch when the match was decided
      */
    void match(int input, int output, int timeSlot);

    /**
      * Runs the batch optimization in the last time slots where each output ports tries
      * to match its free/empty time slots in the batch using condtionalApprovals
      */
    void endOfBatchOptimization();

  public:
    ~SB_QPS_Conditional() override = default;

    /**
      * Schedules the arrived packets for the current time slot
      */
    void schedule(const IQSwitch *iqSwitch) override;

    /**
      * Initializes variable values and/or performs any other initial tasks
      */
    void init(const IQSwitch *iqSwitch) override;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override;

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os) const override;
    

    /**
     * Reserved for future use
     */
    void dumpStats(std::ostream &os){};

}; ///< SB_QPS_Conditional

/** @brief Run QPS-1 for each time slot but accept multiple proposals
 *
 *  For each time slot, QPS-1 is run where each output port accepts
 *  up to maxAccepts number of proposals from input ports and tries to
 *  schedule all of them in the free time slots.
 *
 */
class SB_QPS_Fixed :public BatchScheduler{
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
     * @param debug_mode if true, more information is printed to terminal for debugging
     *
     */
    SB_QPS_Fixed(std::string name, int numInputs, int numOutputs, int batchSize, std::mt19937::result_type seed, int maxAccepts, bool debug_mode);

    using bitmap_t = std::bitset<128>;

    /// Value that represents empty schedule or not yet matched
    static const int NULL_SCHEDULE;

    /// Random number generation seed
    std::mt19937::result_type seed;
    /// Random number generation engine
    std::mt19937 eng{ std::random_device{}() };

    /// Queue of packets present at each input port from which output port will be sampled
    std::vector<saber::Queue_Sampler> inputQueues;

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
      * Runs the logic for SB-QPS for a particular time slot to create current schedule
      * and update previous schedules if needed
      *
      * @param iqSwitch instance of IQSwitch which has packet arrival information
      * @param timeSlot time slot in the batch for which this round of SB-QPS is run
      */
    virtual void qps(const IQSwitch* iqSwitch, size_t timeSlot);

  public:
    ~SB_QPS_Fixed() override {
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
    void dumpStats(std::ostream& os){};

}; ///< SB_QPS_FIXED

} ///< namespace saber

#endif ///< SB_QPS_H
