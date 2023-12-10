#include "qp_sampler.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>

namespace saber {
// base class Queue_Sampler
Queue_Sampler::Queue_Sampler(int num_outputs):
                _num_outputs(num_outputs), _state(NONE)
{
	_leaf_num = compute_leaf_num(num_outputs);
	assert(_num_outputs>0 && _leaf_num >= _num_outputs);
	_queue.resize(_leaf_num * 2, 0);
}

int Queue_Sampler::compute_leaf_num(int num_outputs)
{
	return (int)(1u << ((uint)(std::ceil(std::log2((double)num_outputs)))));
}


void Queue_Sampler::update(size_t pos, int delta)
{
	assert(_queue[pos] + delta >= 0 && "Now we only support non-negative numbers");
	_queue[pos] += delta;
	if (pos == 1) return;  //root
	update(pos / 2, delta);
}

int Queue_Sampler::sample(std::mt19937 &eng, Sample_Type tp)
{
    if (tp == NONE) {return -1;}
    if (tp != _state){build_tree(tp);}

    if (_queue[1] == 0) return -1;
    std::uniform_int_distribution<int> dist(0, _queue[1] - 1);
    int idx = dist(eng);
    int out = search(1, idx) - _leaf_num;

#ifdef DEBUG
	std::cerr << "random : " << idx << "\n";
	std::cerr << "out : " << out << "\n";
	std::cerr << "bst : " << _queue << "\n";
	std::cerr << "VOQ[i][j] : " << _queue[out+_leaf_num] << "\n";
#endif
	assert(out >= 0 && out < _num_outputs);
	return out;
}

void Queue_Sampler::build_tree(Sample_Type tp) {
    if (tp == NONE) {return;}

    //clear all former nodes
    for (int i=1; i<_leaf_num; ++i){_queue[i] = 0;}
    for (int i=_leaf_num; i<2*_leaf_num; ++i){
        int delta = _queue[i];
        // treat leaf as 0/1 value for uniform sampling
        delta = (tp==UNIFORM && delta>=1)?1:delta;
        update(i/2, delta);
    }
}

int Queue_Sampler::search(int cur, int idx) const
{
	int left = 2 * cur;
	int right = 2 * cur + 1;
	if (left >= _leaf_num * 2) return cur;  // leaf node reached
	if (idx < _queue[left]) return search(left, idx); // left branch
	else return search(right, idx - _queue[left]);
}

int Queue_Sampler::get_queue_length() const
{
    int result = 0;
    for (int i=_leaf_num; i<2*_leaf_num; ++i){
      result += _queue[i];
    }
    return result;
}

int Queue_Sampler::get_packet_number(int dst) const
{
	assert(dst >= 0 && dst < _num_outputs);
	return _queue[dst + _leaf_num];
}

void Queue_Sampler::reset()
{
	std::fill(_queue.begin(), _queue.end(), 0);
}

void Queue_Sampler::add_item(int dst)
{
  assert(dst >= 0 && dst < _num_outputs);
  _state = NONE;
  ++_queue[_leaf_num + dst];
}

void Queue_Sampler::remove_item(int dst)
{
  assert(dst >= 0 && dst < _num_outputs);
  _state = NONE;
  --_queue[_leaf_num + dst];
  assert(_queue[_leaf_num + dst]>=0);
}

// cout overload
std::ostream& operator<<(std::ostream& os, const saber::Queue_Sampler &qp)
{
    for (size_t idx = qp._leaf_num; idx < 2*qp._leaf_num; ++idx)
            os << std::setw(3) <<  qp._queue[idx];
    return os;
}
}// namespace saber