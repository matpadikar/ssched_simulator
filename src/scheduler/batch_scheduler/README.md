# Batch Scheduling

## Common Batch Scheduler Class

**This class definition contains common variables and methods for all batch scheduling strategies and is meant to be used as a base class. This class itself is derived from the Scheduler class and calls its constructor by providing the number of input and output ports.**

### Class variables
1. Current batch and previous batch relative time which can be used to keep track of time slots (frame ids) of a particular frame. A new set of packet arrivals is processed every time slot.
2. Boolean for whether the batch size is fixed or not. Batch size specifies the current batch size. Initial batch size specifies the batch size that we started with and the value to which we can reset to.
3. List of schedules for all time slots in the current batch. Similarly, another list of schedules for all time slots in the previous batch.

### Class methods
1. Getter functions to get the value of current batch size and whether batch size is fixed or not.
2. Schedule function that has the logic for creating the schedule using the particular strategy.
3. Display function to print to the terminal current values of the class variables.
4. Reset function to reset variables to initial values.

## Common SB-QPS (Small Batch - Queue Proportional Sampling) Class Definition

**There are many classes defined for various SB-QPS strategies. All these classes share similar variable and method definitions as described below. However, they differ mainly in the implementation of the qps function that has logic for the specific strategy. The specific strategies are described in later sections.**

### Class variables
1. Seed to be used for random number generation.
2. Primary data structure for maintaining length of VOQ for each input-output pair.
3. Auxiliary data structure using binary search tree for queue proportional sampling of an output port.
4. Input match bitmaps which stores information about which time slots of the current batch a given input port is already matched with some output port. Similarly output match bitmaps are also stored.
5. Input availability which defines for how many previous time slots in the batch a given input is available i.e., not yet matched. Similarly output availability is also stored.

### Class methods
1. Function to handle packet arrivals and packet departures which update both the primary and auxiliary data structure using the information of arrived packets in that time slot and subsequently packets departing after scheduling.
2. Sampling Function to sample an output port for a given input port in a queue proportional way using the auxiliary data structure.
3. Getter function to get total queue length for a given input port.
5. Schedule function which calls individual functions handling scheduling tasks in the needed order.
4. qps function which has all the specific logic of creating schedules for the entire batch using the specific strategy and data from primary and auxiliary data structures.
5. Reset functions to reset the bitmaps, primary and auxiliary data structures to zero/null values.
6. Display function to display current value of the seed and auxiliary data structure (binary search tree used for sampling).

## SB (Small Batch) QPS Strategies

### 1. SB-QPS Basic
This strategy is to just run QPS-1 in each time slot to create a schedule for that time slot. **There is no actual batch optimization being done so this is equivalent to just running QPS-1 without any batching.** This can act as base case over which we can track improvements by using batching strategies defined in following sections.

### 2. SB-QPS Half Half-Oblivious
For first half of the time slots, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest (in terms of VOQ size) input port which proposed to it.

**For second half of the time slots**, QPS-1 is run to set the schedule for current time slot. **Additionally, each output port also tries to schedule the second largest input port which proposed to it, for an earlier available/free/empty time slot if any.**

### 3. SB-QPS Half Half-Availability-Aware
For the first half of the time slots, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest (in terms of VOQ size) input port which proposed to it.

**For second half of the time slots**, QPS-1 is run to set the schedule for current time slot. **Additionally, each output port also tries to schedule all the remaining input ports which proposed to it, for any earlier available/free/empty time slots.**

### 4. SB-QPS Adaptive
For each time slot, QPS-1 is run to set the schedule for the current time slot. **Additionally, each output port tries to schedule a `maxAccept-1` number of input ports amongst the remaining proposals, for any earlier available/free/empty time slots. This `maxAccept` value is directly proportional to the available/free/empty time slots and inversely proportional to the number of remaining batch time slots.**

### 5. SB-QPS Half Half-MI (Multi-Iteration)
For the first half of the time slots, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest (in terms of VOQ size) input port which proposed to it.

**For the second half of the time slots, QPS-3 (i.e., 3 iterations of QPS) is run to set the schedule for current time slot. Additionally in each iteration, each output port also tries to schedule the second largest input port which proposed to it, for an earlier available/free/empty time slot if any.**

### 6. SB-QPS Three Third MI (Multi-Iteration)
For the first half of the time slots, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest (in terms of VOQ size) input port which proposed to it.

For first one third of the second half of the time slots, QPS-2 (i.e., 2 iterations of QPS) is run to set the schedule for current time slot. Additionally in each iteration, each output port also tries to schedule the second largest input port which proposed to it, for an earlier available/free/empty time slot if any.

**For the last one third of the time slots, QPS-3 (i.e., 3 iterations of QPS) is run to set the schedule for current time slot. Additionally in each iteration, each output port also tries to schedule three more input port which proposed to it, for any earlier available/free/empty time slots.**

### 7. SB-QPS Half Half-MA (Multi-Accept)
For the first half of the time slots, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest (in terms of VOQ size) input port which proposed to it.

**For second half of the time slots**, QPS-1 is run to set the schedule for current time slot. **Additionally, each output port also tries to schedule `maxAccept-1` remaining input ports which proposed to it, for any earlier available/free/empty time slots.** 

Currently `maxAccept` is set to 2, which is effectively SB-QPS Half Half-Oblivious.

### 8. SB-QPS Half Half-MA-MI (Multi-Accept-Multi-Iteration)
For the first half of the time slots, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest (in terms of VOQ size) input port which proposed to it.

**For the second half of the time slots, QPS-3 (i.e., 3 iterations of QPS) is run to set the schedule for current time slot. Additionally in each iteration, each output port also tries to schedule `maxAccept-1` remaining input port which proposed to it, for any earlier available/free/empty time slots.**

Currently `maxAccept` is set to 2, which is effectively SB-QPS Half Half-MI (Multi-Iteration).

### 9. SB-QPS Conditional
For each time slot, just QPS-1 is run without doing any batch optimizations to set the schedule for that time slot, where each output port accepts the first largest input port (in terms of VOQ size) which proposed to it.

**At the last time slot, each output port tries to fill its earlier empty/free time slots by randomly/uniformly choosing an input port from the list of second largest input ports that had proposed to it. This entire process of QPS-1 and filling of earlier free slots is repeated for a fixed number of iterations**, the only difference being that input ports now propose to an uniformly/randomly chosen output port rather than proposing to an output port in a queue proportional manner.

### 10. SB-QPS Fixed
**For each time slot, output ports accept a `cutoff`/`maxAccept` number of input ports that proposed to them. Then each output port tries to schedule all the accepted input ports in the time slots that are available/free in the batch, starting from the largest (in terms of VOQ size) input port and the earliest time slot.**

## SW (Sliding Window) QPS Strategies

### 1. SW-QPS Basic
**For each time slot, output ports accept a `cutoff`/`maxAccept` number of input ports that proposed to them. Then each output port tries to schedule all the accepted input ports in the time slots that are available/free in the entire sliding window, starting from the largest (in terms of VOQ size) input port and the current time slot (which wraps around to the start of the sliding window).**

### 2. SW-QPS Best Fit
For each time slot, output ports accept all the input ports that proposed to them. Then each output port creates a matching by trying to schedule all the accepted input ports in the time slots that are available/free in the entire sliding window, starting from the largest (in terms of VOQ size) input port and the current time slot (which wraps around to the start of the sliding window).

**The matching using QPS for each output port at each time slot is improved by running maximum cardinality bipartite matching using the QPS matching as initial matching.**

### 3. SW-QPS RR (Round-Robin)
For each time slot, output ports accept a `cutoff`/`maxAccept` number of input ports that proposed to them. Then each output port tries to schedule all the accepted input ports in the time slots that are available/free in the entire sliding window, **starting from the input port chosen in a round-robin manner** and the current time slot (which wraps around to the start of the sliding window).**

## Reference papers
1. [Queue-Proportional Sampling: A Better Approach to Crossbar
Scheduling for Input-Queued Switches. Gong et al.](https://dl.acm.org/doi/10.1145/3084440)
2. [Sliding-Window QPS (SW-QPS): A Perfect Parallel Iterative
Switching Algorithm for Input-Queued Switches. Meng et al.](https://arxiv.org/abs/2010.08620)
