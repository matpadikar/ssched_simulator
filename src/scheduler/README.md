# Proposing strategy

## Queue-Proportional Sampling

### Class variables
1. Current acceptance policy being used by QPS. The expected values are longest_first, earliest_first, random, shortest_first and smallest_first. The default policy is longest_first.
2. Boolean for whether the sampling is done without VOQ replacement for each iteration.
3. 2D vector used to store the binary search tree (BST). The BST stores the length of the VOQs for each input port in its leaves.
4. Offset of the BST pointing to the start of leaves.

### Class methods
1. Schedule function that has the logic for creating the schedule using QPS.
2. Display function to print to the terminal current values of the class variables.
3. Reset function to reset variables to initial values.

### QPS strategy
This algorithm consists of two steps.

Step 1:
A packet is sampled out of all the packets currently queued at the input port, uniformly at random. If there are a total of m packets, across all VOQs (total N) at the input port, then each packet is sampled with the probability 1/m. For example- if the particular VOQ has length n, then it will have one of its packets sampled with probability n/m. 

Step 2:
Suppose a packet is proposed using the above step. First, we find out which VOQ the packet belongs to. The input port for that VOQ proposes to the corresponding output port with its queue length. If this proposal is successful, then the pair of the input and outport port is a part of the actual matching. The actual serviced packet is the HOL packet of the VOQ from where the original packet was sampled. Note that the departure packet is the HOL packet, and not the sampled packet. 

### Time Complexity
O(1) computational complexity per port.

## iSLIP

### Class variables
1. 2 separate vectors to keep track of the accept and grant pointer status of each input and output port respectively.
2. Data structure which manages the queue status for each input and output port pairing. Its value is set to true if there exists a packet queued at a particular input port for a particular output port.
3. Number of iterations for each round in iSLIP

### Class methods
1. Schedule function that has the logic for creating the schedule using QPS.
2. Display function to print to the terminal current values of the class variables.
3. Reset function to reset variables to initial values.

### iSLIP scheduling algorithm
iSLIP avoids the randomization present in PIM, by providing fairness by choosing a 'winner' in a round-robin fashion.

In iSLIP, every output port maintains a grant pointer g which is initially set to the first input port. When an output port has to choose between multiple input requests, it chooses the lowest number greater than or equal to g. Every input port also maintains a similar pointer, called the accept pointer a. In the case of the input port, it chooses the lowest output port number greater than or equal to a. In case an output port is matched with input port X, then the g pointer for that port is incremented to X + 1 in a circular order. This algorithm can be divided into 3 steps.

Step 1: Request
Each input port sends requests to each output port for which it has a packet destined.

Step 2: Grant
If an output port receives a single request, it sends a grant back to that port.
If an output port receives multiple requests, it chooses the lowest input port according to g, and sends a grant back to it.

Step 3: Accept
Similar to the grant phase, every input port chooses the lowest output port according to a, and sends an accept to that port.

These 3 steps are repeated multiple times in what are called the iterations of a round. In the final iteration, when every input port that can send a packet under iSLIP algorithm is ready, the actual data transfer occurs in Step 3. 

## Reference papers
1. [Queue-Proportional Sampling: A Better Approach to Crossbar Scheduling for Input-Queued Switches. Gong et al.](https://dl.acm.org/doi/10.1145/3084440)
2. [The iSLIP Scheduling Algorithm for Input-Queued Switches. N. McKeown.](https://ieeexplore.ieee.org/document/769767)