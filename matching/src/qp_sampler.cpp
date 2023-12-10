#include "qp_sampler.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "timer.hpp"

namespace saber {
// base class Queue_Sampler
Queue_Sampler::Queue_Sampler(int num_outputs):
                _num_outputs(num_outputs){
	_leaf_num = compute_leaf_num(num_outputs);
	assert(_num_outputs>0 && _leaf_num >= _num_outputs);
	_queue.resize(_leaf_num * 2, 0);
}

Queue_Sampler::Queue_Sampler(const std::vector<int>& weights):
_num_outputs(weights.size()){
  _leaf_num = compute_leaf_num(_num_outputs);
  assert(_num_outputs>0 && _leaf_num >= _num_outputs);
	_queue.resize(_leaf_num * 2, 0);
  // copy from weights to queue
  std::copy(weights.begin(), weights.end(), _queue.begin() + _leaf_num);

  build(1);
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

int Queue_Sampler::sample(std::mt19937 &eng)
{
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

void Queue_Sampler::build_tree() {
   //for (int i=_leaf_num; i<2*_leaf_num; ++i){
   //     int delta = _queue[i];
   //     update(i/2, delta);
  //  }
  build(1);
}

int Queue_Sampler::build(int i){
  if (i >= _leaf_num) return _queue[i];
  else {
    _queue[i] = build(i * 2) + build(2 * i + 1);
    return _queue[i];
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
/*
void Queue_Sampler::add_item(int dst)
{
  assert(dst >= 0 && dst < _num_outputs);
  ++_queue[_leaf_num + dst];
}*/

void Queue_Sampler::remove(int dst)
{
  assert(dst >= 0 && dst < _num_outputs);
  update((_leaf_num + dst) / 2, -_queue[_leaf_num + dst]);
  _queue[_leaf_num + dst] = 0;
}

// cout overload
std::ostream& operator<<(std::ostream& os, const saber::Queue_Sampler &qp)
{
    for (size_t idx = qp._leaf_num; idx < 2*qp._leaf_num; ++idx)
            os << std::setw(3) <<  qp._queue[idx];
    return os;
}

Queue_Sampler& Queue_Sampler::operator=(const std::vector<int>& weights){
  assert(_num_outputs ==  weights.size());

  std::copy(weights.begin(), weights.end(), _queue.begin() + _leaf_num);
  

  build(1);
  return *this;
}
///////////////////////////////////////////////////////////////////////

Max_Propose::Max_Propose(int num_outputs):
                _num_outputs(num_outputs){
	_leaf_num = compute_leaf_num(num_outputs);
	assert(_num_outputs>0 && _leaf_num >= _num_outputs);
	_queue.resize(_leaf_num * 2, 0);
}

Max_Propose::Max_Propose(const std::vector<int>& weights):
_num_outputs(weights.size()){
  _leaf_num = compute_leaf_num(_num_outputs);
  assert(_num_outputs>0 && _leaf_num >= _num_outputs);
	_queue.resize(_leaf_num * 2, 0);
  // copy from weights to queue
  std::copy(weights.begin(), weights.end(), _queue.begin() + _leaf_num);

  build(1);
}

int Max_Propose::compute_leaf_num(int num_outputs)
{
	return (int)(1u << ((uint)(std::ceil(std::log2((double)num_outputs)))));
}


void Max_Propose::update(size_t pos)
{
  while(pos > 0){
     _queue[pos] = std::max(_queue[2 * pos], _queue[2 * pos + 1]);
     pos /= 2;
  }
}

int Max_Propose::sample()
{
    if (_queue[1] == 0) return -1;
    int out = search(1) - _leaf_num;

#ifdef DEBUG
	std::cerr << "random : " << idx << "\n";
	std::cerr << "out : " << out << "\n";
	std::cerr << "bst : " << _queue << "\n";
	std::cerr << "VOQ[i][j] : " << _queue[out+_leaf_num] << "\n";
#endif
	assert(out >= 0 && out < _num_outputs);
	return out;
}

void Max_Propose::build_tree() {
   //for (int i=_leaf_num; i<2*_leaf_num; ++i){
   //     int delta = _queue[i];
   //     update(i/2, delta);
  //  }
  build(1);
}

int Max_Propose::build(int i){
  if (i >= _leaf_num) return _queue[i];
  else {
    _queue[i] = std::max(build(i * 2), build(2 * i + 1));
    return _queue[i];
  }
}

int Max_Propose::search(int cur) const
{
	int left = 2 * cur;
	int right = 2 * cur + 1;
	if (left >= _leaf_num * 2) return cur;  // leaf node reached
	if (_queue[left] >= _queue[right]) return search(left); // left branch
	else return search(right);
}

int Max_Propose::get_queue_length() const
{
    int result = 0;
    for (int i=_leaf_num; i<2*_leaf_num; ++i){
      result += _queue[i];
    }
    return result;
}

int Max_Propose::get_packet_number(int dst) const
{
	assert(dst >= 0 && dst < _num_outputs);
	return _queue[dst + _leaf_num];
}

void Max_Propose::reset()
{
	std::fill(_queue.begin(), _queue.end(), 0);
}
/*
void Queue_Sampler::add_item(int dst)
{
  assert(dst >= 0 && dst < _num_outputs);
  ++_queue[_leaf_num + dst];
}*/

void Max_Propose::increment(size_t dst, int delta)
{
  assert(dst >= 0 && dst < _num_outputs);
  _queue[_leaf_num + dst] += delta;
  update((_leaf_num + dst) / 2);
}
bool Max_Propose::is_empty() const{
  return std::all_of(_queue.begin(), _queue.end(), [](int i) { return i==0; });
}

// cout overload
std::ostream& operator<<(std::ostream& os, const saber::Max_Propose &qp)
{
    for (size_t idx = qp._leaf_num; idx < 2*qp._leaf_num; ++idx)
            os << std::setw(3) <<  qp._queue[idx];
    return os;
}

Max_Propose& Max_Propose::operator=(const std::vector<int>& weights){
  assert(_num_outputs ==  weights.size());

  std::copy(weights.begin(), weights.end(), _queue.begin() + _leaf_num);
  build(1);
  return *this;
}

QuantizedProposer::QuantizedProposer(const std::vector<int>& weights, int pool_size): 
_pool(pool_size), _dist(0, pool_size - 1), _pool_size(pool_size), _output_num(weights.size()),
_good(true)
{
  *this = weights;
  //std::cout << _pool.size();
}

QuantizedProposer& QuantizedProposer::operator=(const std::vector<int>& weights)
{
  StopWatch tt;
  double idx = 0.0;
  int cur = 0;
  double sum = static_cast<double> (std::accumulate(weights.begin(), weights.end(), 0));
  if (sum == 0) {
    _good = false;
    return *this;
  }

  _pool.assign(_pool_size, 0);
  for(int i = 0; i < _output_num; ++i){
    idx += weights[i] / sum;
    int end = static_cast<int> (idx * _pool_size);
    assert(end <= _pool_size);
    for (; cur < end; ++cur){
      _pool[cur] = i;
    }
  }
  for (; cur < _pool_size; ++cur){
    _pool[cur] = weights.size() - 1;
  }

  _good = true;
  //std::cout << tt.peek() << std::endl;
  //std::cout << _pool.size()  << "\t" << weights.size() << std::endl;
  return *this;
}

QuantizedProposer::QuantizedProposer(int pool_size, int output_num):
_pool(pool_size), _dist(0, pool_size - 1), _pool_size(pool_size), _output_num(output_num),
_good(false) {

}

int QuantizedProposer::sample(std::mt19937 &eng) {
  static unsigned ratio = 0xffffffffu / _pool_size;
  int idx = eng() / ratio;
  if (idx >= _pool_size) idx = _pool_size - 1;
  if (_good) return _pool[idx];
  else return -1;
}

}// namespace saber
