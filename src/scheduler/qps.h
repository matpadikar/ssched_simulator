//   qps.h header file

#ifndef QPS_H
#define QPS_H

#include "scheduler.h"

namespace saber {

/**
 * Base class for Queue Proportional Sampling proposing strategy
*/
class QPS : public RandomizedScheduler {

  /// Allow SchedulerFactory class to access protected variables and methods
  friend class SchedulerFactory;

protected:
  using bst_t = std::vector<int>;
  int _iterations{1};

  /// Accept policy being used for QPS
  std::string _accept_policy{"longest_first"};
  bool _without_replacement{false};
  int _left_start{-1};

  /// Binary Search Tree used to store the lengths of input port VOQs in its leaves
  std::vector<bst_t> _bst;

  /**
   * Protected constructor for QPS
   * 
   * @param name name for an instance of the class
   * @param num_inputs number of input ports in the switch
   * @param num_outputs number of output ports in the switch
   * @param seed result type for the random number generator
   * @param iterations number of iterations for which to run QPS
   * @param accept_policy packet acceptance policy. Can be longest_first,
   *                        earliest_first, random, shortest_first, smallest_first
   * @param without_replacement boolean to determine if BST should be updated
   *                        without replacement, i.e, if an output port rejects a packet
   *                        it is not considered in the next iteration
  */
  QPS(std::string name, int num_inputs, int num_outputs,
      std::mt19937::result_type seed, int iterations, std::string accept_policy,
      bool without_replacement);

  /**
   * Updates BST with arrival packets
   * 
   * @param iqSwitch instance of IQSwitch which will provide incoming packet information
  */
  void handleArrivals(const IQSwitch *iqSwitch);

  /**
   * Updates BST if an input has been matched and there is at least 1 packet that
   * wants to go from a particular input port to a particular output port
   * 
   * @param iqSwitch instance of IQSwitch 
  */
  void handleDepartures(const IQSwitch *iqSwitch);

  /**
   * Sample a random output port from the input port VOQs
   * 
   * @param source input port number
   * @returns int representing output port number
  */
  inline int sampling(int source);

  /**
   * Getter function for the queue length of the input port
   * 
   * @param source input port number
   * @returns int representing the queue length
  */
  inline int getQueueLength(int source);

  /**
   * Removes VOQ in case without_replacement policy followed and 
   * outport port rejected packet
   * 
   * @param source input port number
   * @param destination output port number
   * @returns root of BST
  */
  inline int removeVOQ(int source, int destination);

  /**
   * Restores VOQs after scheduling packets
   * 
   * @param restore_pairs vector of pairs (input port, outport port) with root
   * of BST
  */
  inline void restoreVOQ(
      const std::vector<std::pair<std::pair<int, int>, int>> &restore_pairs);

public:
  ~QPS() override = default;

  /**
   * Resets all the variable values in BST to 0
  */
  void reset() override;

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
   * Prints out current values of the variables
  */
  void display(std::ostream &os) const override;
};
} // namespace saber

#endif // QPS_H
