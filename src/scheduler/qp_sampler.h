// qp_sampler.h header file
// Queue Length Proportional Sampler For a Particular Input Port
//

#ifndef QP_SAMPLER_H
#define QP_SAMPLER_H

#include <vector>
#include <random>

namespace saber {
class Queue_Sampler{
  friend std::ostream& operator<<(std::ostream& os, const saber::Queue_Sampler& qs);
public:
  enum Sample_Type{
    NONE, QPS, UNIFORM
  };
protected:
  /**  A binary tree storing the number of packets classified by destination port.
	*  root #1: total number of packets for this input port
	*  leaf #x (_leaf_num <= x <= 2*_leafnum-1): number of packets with dst = #(x-_leaf_num) port (qps)
	*  intermediate #x (2<= x <= _leaf_num-1): sum of values of its both children #(2*x) and #(2*x+1) treat leaf children as 0/1 valued for uniform mode
	*  For example:       15
	*                  /      \
	*                 8        7
	*               /   \    /   \
	*              3     5  1     4
	*/
  std::vector<int> _queue;
  int _leaf_num;
  const int _num_outputs;
  Sample_Type _state;
  /**
  *	Returns min x
      *	Such that: x is a power of 2, and x >= queue_len
      *	For example: n = 3 -> x = 4
      *		     n = 4 -> x = 4
      *		     n = 5 -> x = 8
      */
  static int compute_leaf_num(int queue_len);
  void update(size_t dst, int delta);
  int search(int current, int idx) const;
  void build_tree(Sample_Type tp);
public:

  explicit Queue_Sampler(int num_outputs);
  void add_item(int dst);
  // remove one packet from queue
  void remove_item(int dst);
  // samples one output port proportional to queue length from a random generator eng
  int sample(std::mt19937 &eng, Sample_Type tp);
  // get total input queue length
  int get_queue_length() const;
  // get packet number for a given destination
  int get_packet_number(int dst) const;
  // printing overload

  void reset();
};
} // namespace saber
#endif //QP_SAMPLER_H
