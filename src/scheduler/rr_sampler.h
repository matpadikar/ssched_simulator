// rr_sampler.h header file
// Round Robin Sampler For a Particular Input Port
//

#ifndef RR_SAMPLER_H
#define RR_SAMPLER_H

#include <vector>
#include <deque>
#include <random>

namespace saber {
class Round_Robin_Sampler{
  friend std::ostream& operator<<(std::ostream& os, const saber::Round_Robin_Sampler& qs);

protected:
/**  A queue that contains all currently non-empty VOQs 
	*/
  std::deque<int> _rrqueue;
/* Virtual Output Queues*/
  std::vector<int> _queue;
  const int _num_outputs;
public:

  explicit Round_Robin_Sampler(int num_outputs);
  void add_item(int dst);
  // remove one packet from queue
  void remove_item(int dst);
  // samples one output port proportional to queue length from a random generator eng
  int sample();
  // get total input queue length
  int get_queue_length() const;
  // get packet number for a given destination
  int get_packet_number(int dst) const;
  // printing overload

  void reset();
};
} // namespace saber
#endif //RR_SAMPLER_H
