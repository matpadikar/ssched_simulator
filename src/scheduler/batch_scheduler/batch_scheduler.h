
#ifndef BATCH_SCHEDULER_H
#define BATCH_SCHEDULER_H

#include <scheduler/scheduler.h>

namespace saber {

/**
 * Macros for batch-related parameters
 */
#ifndef MAX_BATCH_SIZE
#define MAX_BATCH_SIZE 32768
#endif

#ifndef DEF_BATCH_SIZE
#define DEF_BATCH_SIZE 4096
#endif

#ifndef BATCH_SIZE_BLOCK
#define BATCH_SIZE_BLOCK 128
#endif


/**
 * Base class to be inherited by all classes that follow batch scheduling strategy
 */
class BatchScheduler : public Scheduler {

  /// Allow SchedulerFactory class to access protected variables and methods
  friend class SchedulerFactory;

  protected:
    /**
     * Protected constructor to be used by derived classes which inherit this class
     *
     * @param name name for an instance of the class
     * @param numInputs number of input ports in the switch
     * @param numOutputs number of output ports in the switch
     * @param batchSize number of time slots for which scheduling will be done together
     * @param isBatchSizeFixed whether or not to allow batch size change after initialization
     *
     */
    BatchScheduler(const std::string &name, int numInputs, int numOutputs, int batchSize, bool isBatchSizeFixed);

    /// current batch relative time
    size_t _cf_rel_time;
    /// previous batch relative time
    int _pf_rel_time;

    /// initial value for batch size
    const size_t initialBatchSize;
    /// current batch size
    size_t batchSize;
    /// whether or not to allow batch size change after initialization
    const bool isBatchSizeFixed;

    /// schedules for current batch
    std::vector<std::vector<int> > schedules;
    /// schedules for previous batch
    std::vector<std::vector<int> > schedules_pre;

  public:
    ~BatchScheduler() override = default;

    /**
      * Getter function for batchSize
      *
      * @returns a size_t (which can treated as int for most part) representing batch size
      */
    size_t getBatchSize() const override { return batchSize; }

    /**
      * Getter function for isBatchSizeFixed
      *
      * @returns a boolean representing whether the batch size is fixed or not
      */
    bool getIsBatchSizeFixed() const { return isBatchSizeFixed; }

    /**
      * Initializes variable values and/or performs any other initial tasks
      *
      * @param iqSwitch instance of IQSwitch which will provide incoming packet information
      */
    void init(const IQSwitch *iqSwitch) override = 0;

    /**
      * Creates the schedule for the current time slot
      *
      * @param iqSwitch instance of IQSwitch which will provide packet arrival information
      */
    void schedule(const IQSwitch *iqSwitch) override = 0;

    /**
      * Resets all the variable values to initial values
      */
    void reset() override ;

    /**
      * Prints out current values of the variables
      */
    void display(std::ostream &os)  const override ;

    /**
      * Reserved for future use
      */
    void dumpStats(std::ostream &os) {};
};

} ///< namespace saber

#endif ///< BATCH_SCHEDULER_H
