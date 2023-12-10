// maximal_scheduler.h header file

#ifndef MAXIMAL_SCHEDULER_H
#define MAXIMAL_SCHEDULER_H

#include "scheduler.h"

namespace saber {
/**
 * Base class for Randomized Maximal Matching Scheduler
*/
class RandomizedMaximalScheduler : public RandomizedScheduler {

  /// Allow SchedulerFactory class to access protected variables and methods
  friend class SchedulerFactory;

protected:

  /// Records edges between input and outport port
  using Edge = std::pair<int, int>;
  std::vector<Edge> _edges{};

  /// Total number of edges
  int _edge_num{0};

  /// Record edge numbers
  std::vector<std::vector<int>> _index;

  /**
   * Protected constructor for Randomized Maximal Matching Scheduler
   * 
   * @param name name for an instance of the class
   * @param num_inputs number of input ports in the switch
   * @param num_outputs number of output ports in the switch
   * @param seed result type for the random number generator
  */
  RandomizedMaximalScheduler(std::string name, int num_inputs, int num_outputs,
                             std::mt19937::result_type seed);

  /** 
   * This function only inserts an edge when the edge between
   * @source and @destination does not exist. If it exists, this function
   * will do nothing.
   *
   * @param source source port number
   * @param destination destination port number
   */
  void insert(int source, int destination);

  /** 
   * This function first checks whether the edge between @source and
   * @destination exists. If it exists, then we remove the edge. Otherwise, it
   * does nothing.
   *
   * @param source source port number
   * @param destination destination port number
   */
  void erase(int source, int destination);

  /**
   * Gets the arrival packets from the IQSwitch and calls the insert function
   * 
   * @param iqSwitch instance of IQSwitch which will provide incoming packet information
  */
  void handleArrivals(const IQSwitch *iqSwitch);

  /**
   * If an input has been matched and there is exactly 1 packet that
   * wants to go from a particular input port to a particular output port
   * then call the erase function
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void handleDepartures(const IQSwitch *iqSwitch);

public:
  ~RandomizedMaximalScheduler() override = default;

  /**
   * Creates the schedule for the current time slot
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void schedule(const IQSwitch *iqSwitch) override;

  /**
   * Initializes variable values and/or performs any other initial tasks
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void init(const IQSwitch *iqSwitch) override;

  /**
   * Resets all the variable values to initial values
  */
  void reset() override;
}; // RandomizedMaximalScheduler


/**
 * Base class for iSLIP
 * Class for <a href="https://dl.acm.org/citation.cfm?id=310896">iSLIP</a>
*/
class iSLIP : public Scheduler {

  /// Allow SchedulerFactory class to access protected variables and methods
  friend class SchedulerFactory;

protected:
  int _iterations;
  std::vector<int> _grant_pointers;
  std::vector<int> _accept_pointer;

  std::vector<std::vector<bool>> _queue_status;

  /**
   * Protected constructor for iSLIP
   * 
   * @param name name for an instance of the class
   * @param num_inputs number of input ports in the switch
   * @param num_outputs number of output ports in the switch
   * @param iterations number of iterations in iSLIP
   * @param grant_pointers list which records the input port granted by an 
   *                       output port
   * @param accept_pointer list which records the output port accepted by an
   *                       input port
  */
  iSLIP(const std::string &name, int num_inputs, int num_outputs,
        int iterations, const std::vector<int> &grant_pointers,
        const std::vector<int> &accept_pointer);

  /**
   * Handles the arrival packets from the IQSwitch and updates the queue status
   * for iSLIP
   * 
   * @param iqSwitch instance of IQSwitch which will provide incoming packet 
   *                  information
  */
  void handleArrivals(const IQSwitch *iqSwitch);

  /**
   * If an input has been matched and there is exactly 1 packet that
   * wants to go from a particular input port to a particular output port
   * then update the queue status for iSLIP
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void handleDepartures(const IQSwitch *iqSwitch);

public:
  ~iSLIP() override = default;

  /**
   * Resets all the variable values to initial values
  */
  void reset() override;

  /**
   * Prints out current values of the variables
  */
  void display(std::ostream &os) const override;

  /**
   * Creates the schedule for the current time slot
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void schedule(const IQSwitch *iqSwitch) override;

  /**
   * Initializes variable values and/or performs any other initial tasks
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void init(const IQSwitch *iqSwitch) override;
};
} // namespace saber

#endif // MAXIMAL_SCHEDULER_H
