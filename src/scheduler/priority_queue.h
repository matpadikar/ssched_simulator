// qp_sampler.h header file
// Queue Length Proportional Sampler For a Particular Input Port
//

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
namespace saber {
	/** Priority Queue from a Linear Data Structure
	 *  Slow but easy to implement, used for small data size
	 */
	class Linear_Priority_Queue {
	public:
		const int capacity;
		const int null_symbol;
		// pair<value, priority>
		std::vector<std::pair<int, int> > queue;

		explicit Linear_Priority_Queue(int capacity, int null_symbol = -1);
		void insert(int value, int priority);
		int size() const;
	};
} // namespace saber
#endif //PRIORITY_QUEUE_H
