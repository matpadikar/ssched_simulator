// qp_sampler.h header file
// Queue Length Proportional Sampler For a Particular Input Port
//

#ifndef QP_SAMPLER_H
#define QP_SAMPLER_H

#include <vector>
#include <random>

namespace saber {
class AbstractProposer{
  //explicit virtual AbstractProposer(const std::vector<int>& weights) = 0;
  //virtual AbstractProposer& operator=(const std::vector<int>& weights) = 0;
  virtual int sample(std::mt19937 &eng) = 0;
  virtual void reset() = 0;
};

class Queue_Sampler{
  friend std::ostream& operator<<(std::ostream& os, const saber::Queue_Sampler& qs);
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
  void build_tree();
  int build(int i);
public:

  explicit Queue_Sampler(int num_outputs);
  Queue_Sampler(const std::vector<int>& weights);
  Queue_Sampler& operator=(const std::vector<int>& weights);
  //void add_item(int dst);
  // remove one packet from queue
  void remove(int dst);
  // samples one output port proportional to queue length from a random generator eng
  int sample(std::mt19937 &eng);
  // get total input queue length
  int get_queue_length() const;
  // get packet number for a given destination
  int get_packet_number(int dst) const;
  // printing overload

  void reset();
};

class Max_Propose{
  friend std::ostream& operator<<(std::ostream& os, const saber::Max_Propose& qs);
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
  /**
  *	Returns min x
      *	Such that: x is a power of 2, and x >= queue_len
      *	For example: n = 3 -> x = 4
      *		     n = 4 -> x = 4
      *		     n = 5 -> x = 8
      */
  static int compute_leaf_num(int queue_len);
  void update(size_t pos);
  int search(int current) const;
  void build_tree();
  int build(int i);
public:

  explicit Max_Propose(int num_outputs);
  Max_Propose(const std::vector<int>& weights);
  Max_Propose& operator=(const std::vector<int>& weights);
  //void add_item(int dst);
  // remove one packet from queue
  void increment(size_t dst, int delta);
  // samples one output port proportional to queue length from a random generator eng
  int sample();
  // get total input queue length
  int get_queue_length() const;
  // get packet number for a given destination
  int get_packet_number(int dst) const;
  // printing overload
  bool is_empty() const;
  void reset();
};


// O(1) time per sample, faster than QueueSampler
class QuantizedProposer : public AbstractProposer {
private: 
  std::vector<int> _pool;
  std::uniform_int_distribution<int> _dist;
  int _output_num, _pool_size;
  bool _good;

public:
  QuantizedProposer(int pool_size, int output_num);
  explicit QuantizedProposer(const std::vector<int>& weights, int pool_size);
  QuantizedProposer& operator=(const std::vector<int>& weights);
  int sample(std::mt19937 &eng) override;
  void reset() override{
    _pool.clear();
  }
  bool is_good() const {return _good;}
};


} // namespace saber
#endif //QP_SAMPLER_H
