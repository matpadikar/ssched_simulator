#include "rr_sampler.h"
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>

namespace saber {
Round_Robin_Sampler::Round_Robin_Sampler(int num_outputs):
                _num_outputs(num_outputs)
{
	_queue.resize(_num_outputs, 0);
}



int Round_Robin_Sampler::sample()
{
    if (_rrqueue.size() == 0) return -1; // all VOQs are empty
    int out = _rrqueue[0];
    _rrqueue.pop_front();
    _rrqueue.push_back(out);
	return out;
}


int Round_Robin_Sampler::get_queue_length() const
{
    int result = 0;
    for (auto length: _queue){
      result += length;
    }
    return result;
}

int Round_Robin_Sampler::get_packet_number(int dst) const
{
	assert(dst >= 0 && dst < _num_outputs);
	return _queue[dst];
}

void Round_Robin_Sampler::reset()
{
	std::fill(_queue.begin(), _queue.end(), 0);
    _rrqueue.clear();
}

void Round_Robin_Sampler::add_item(int dst)
{
  assert(dst >= 0 && dst < _num_outputs);
  //if (_queue[dst] == 0) {
  //  _rrqueue.push_back(dst);   // new non-empty VOQ
  //}
 _rrqueue.push_back(dst);
  ++_queue[dst];
}

void Round_Robin_Sampler::remove_item(int dst)
{
  assert(dst >= 0 && dst < _num_outputs && _queue[dst]>0);
  //_rrqueue.pop_front();
_rrqueue.pop_back();
  --_queue[dst];
  //if (_queue[dst] > 0) {
  // _rrqueue.push_back(dst);   //  VOQ still non-empty
 //}
}

// cout overload
std::ostream& operator<<(std::ostream& os, const saber::Round_Robin_Sampler &rr)
{
    for (auto length: rr._queue)
            os << std::setw(3) <<  length;
    return os;
}
}// namespace saber
