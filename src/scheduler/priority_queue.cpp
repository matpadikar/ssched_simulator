#include "priority_queue.h"
#include <cassert>

namespace saber {

Linear_Priority_Queue::Linear_Priority_Queue(int cap, int null_sym) :capacity(cap),
	null_symbol(null_sym),
	queue(capacity, std::pair<int, int>(null_symbol, null_symbol))
{
	assert(capacity >= 0);
}

void Linear_Priority_Queue::insert(int val, int pri)
{
	auto cur = queue.begin();
	std::pair<int, int> tmp;
	while (cur != queue.end() && cur->second >= pri) { ++cur; }
	if (cur != queue.end()) {
		tmp = *cur;
		cur->first = val;
		cur->second = pri;
		++cur;
	}
	while (cur != queue.end() && cur->second != null_symbol)
	{
		std::swap(tmp, *cur);
		++cur;
	}
}

int Linear_Priority_Queue::size() const
{
    int result = 0;
    for(auto cur = queue.begin(); cur!=queue.end() && cur->second!=null_symbol; ++cur){
      ++result;
    }
    return result;
}
} //namespace saber