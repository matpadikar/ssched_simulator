#include "sliding_window_qps.h"
#include "scheduler/circular_generator.h"
#include <lemon/concepts/maps.h>
#include <iomanip>
#include <unordered_map>

namespace saber {

const int Sliding_Window_QPS::NULL_SCHEDULE = -1;
/**
 * Start of implementation for class Sliding_Window_QPS
 */
Sliding_Window_QPS::Sliding_Window_QPS(std::string name, int numInputs, int numOutputs, int batchSize,
	std::mt19937::result_type seed, int maxAccepts, bool batching, bool debug_mode): 
    BatchScheduler(name, numInputs, numOutputs, batchSize, true),
	seed(seed), 
    eng(seed),
    inputMatchBitmap (numInputs),
	outputMatchBitmap(numOutputs), 
    _debug_mode(debug_mode), 
    maxAccepts(maxAccepts), 
    batching(batching),
	outputAvailability(numOutputs, batchSize), 
    inputAvailability (numInputs, batchSize) 
    {
        inputQueues.reserve (numInputs);
        for (size_t i = 0; i < numInputs; ++i){
            inputQueues.emplace_back(_num_outputs);
        }
        _cf_rel_time = 0;

        // generate initial schedulers (used for the first frame)
        for (auto& sched : schedules){
            std::fill(sched.begin(), sched.end(), -1);
        }

        if(_debug_mode){
            _debug_stream.open("debug_dump.txt", std::ofstream::out);
            _debug_stream << "scheduling starts\n";
        } else{
            _debug_stream.setstate(std::ofstream::badbit);
            // disable _debug_stream
        }
    }

void Sliding_Window_QPS::init(const IQSwitch* iqSwitch){
	// reserved
}

void Sliding_Window_QPS::handlePacketArrivals(const IQSwitch* iqSwitch){
	assert(iqSwitch != nullptr);
	auto arrivals = iqSwitch->get_arrivals();
	_debug_stream << "handle arrivals\n";

	for (const auto& srcDestPair : arrivals){
		if (srcDestPair.first == -1){
			break;
        }
		assert(srcDestPair.first >= 0 && srcDestPair.first < _num_inputs);
		assert(srcDestPair.second >= 0 && srcDestPair.second < _num_outputs);
		inputQueues[srcDestPair.first].add_item(srcDestPair.second);
		_debug_stream << "(" << srcDestPair.first << ", " << srcDestPair.second << ")";
	}
	_debug_stream << "\n";
}

void Sliding_Window_QPS::schedule(const saber::IQSwitch* iqSwitch){
	auto timeSlot = (_cf_rel_time % batchSize);
    _debug_stream << "Round: " << std::setw(6) << _cf_rel_time << "\tframe " << std::setw(3) << timeSlot << "\n";
	
	if (batching) dispatchSchedule(timeSlot);
	// qps scheduling
	qps(iqSwitch, timeSlot);

    // dispatch schedule
    if (!batching) dispatchSchedule(timeSlot);
    
	_debug_stream << "buffer state\n" <<std::setw(11) << " ";
	for (int f=0; f<batchSize; ++f){
	    _debug_stream << std::setw(3) << f;
    }
	_debug_stream << "\n";

	for (size_t f=0; f<batchSize; ++f){
	    _debug_stream << "frame " << std::setw(3) << f << ": ";
	    for (int out_port:schedules[f]){
            _debug_stream << std::setw(3) << out_port;
	    }
	    _debug_stream << "\n";
	}
	++_cf_rel_time;
}

void Sliding_Window_QPS::qps(const saber::IQSwitch* iqSwitch, size_t timeSlot){
    /// Refer section 4 of SW-QPS paper https://arxiv.org/pdf/2010.08620.pdf

    assert(isBatchSizeFixed);

    // handle arrivals
    handlePacketArrivals(iqSwitch);

    _debug_stream << "queue state\n" << std::setw(11) << " ";
    for (int f=0; f<_num_inputs; ++f)
    {
        _debug_stream << std::setw(3) << f;
    }
    _debug_stream << "\n";
    for (size_t in=0; in<_num_inputs; ++in){
        _debug_stream << "input " << std::setw(3) << in << ": " << inputQueues[in] << "\n";
    }

    // proposals recieved by each output port
    std::vector<std::vector<int>> out_accepts(num_outputs());

    // shuffle inputs
    std::vector<int> inputs(_num_inputs, 0);
    for (int i = 0; i < _num_inputs; ++i){
        inputs[i] = i;
    }
    std::shuffle(inputs.begin(), inputs.end(), eng);

    // Step 1: Proposing
    for (int i = 0; i < _num_inputs; ++i){
        int in = inputs[i];
        if (getQueueLengthForInput(in) == 0){
            continue; // no packets
        }
        int out = inputQueues[in].sample(eng, Queue_Sampler::QPS); // sampling an output for this input
        assert(inputQueues[in].get_packet_number(out) > 0);
	
        if (out_accepts[out].size()<maxAccepts){ // only accepts up to maxAccepts proposals
            out_accepts[out].push_back(in);
        }
    }

    std::vector<std::pair<int, int>> virtualDepartures; // virtual departures
    // matching for output ports
    std::vector<int> out_match(num_outputs(), NULL_SCHEDULE);
    // Step 2: Accept
    _debug_stream << "accepting\n";
    for (int out = 0; out < _num_outputs; ++out){
        if (out_accepts[out].empty()){  // no proposal received
            continue;
        }

        // sort proposals based on VOQ lengths
        std::sort(out_accepts[out].begin(), out_accepts[out].end(), [=](const int in1, const int in2){
            return inputQueues[in1].get_packet_number(out) > inputQueues[in2].get_packet_number(out);
        });

        for (int in: out_accepts[out]){
            if (outputAvailability[out] == 0){
                if(_debug_mode){
                    _debug_stream << "(" <<  in << ", " << out << ", -)" ;
                    continue;
                } else {
                    break;
                }
            }
            _debug_stream << "(" <<  in << ", " << out << ", ";

            if (inputAvailability[in] == 0){
                continue; // no available ts
            }
            
            // available only when both available
            auto mf = (inputMatchBitmap[in] | outputMatchBitmap[out]);
            bool success = false;
            for (int f : saber::Circular_Generator{(int) timeSlot, (int)batchSize, batching}){
                if (!mf.test(f)){
                    inputMatchBitmap[in].set(f);
                    outputMatchBitmap[out].set(f);
                    --inputAvailability[in];
                    --outputAvailability[out];
                    assert(schedules[f][in] == NULL_SCHEDULE);
                    schedules[f][in] = out;
                    virtualDepartures.emplace_back(in, out);
                    _debug_stream << f << ")";
                    success = true;
                    break; // pay special attention (first fit)
                }
            }
            if (!success){
                _debug_stream << "-)";
            }
        }
    }
    _debug_stream << "\n";

    #ifdef DEBUG
        std::cerr << inputQueues << "\n";
        std::cerr << virtualDepartures << "\n\n";
    #endif

	handlePacketDepartures(virtualDepartures);
}

void Sliding_Window_QPS::dispatchSchedule(int timeSlot){
    // copy schedule to _in_match, output to simulator
    std::copy(schedules[timeSlot].begin(), schedules[timeSlot].end(), _in_match.begin());

    // increment availability for each dispatched packet
    for (auto iter = schedules[timeSlot].begin(); iter != schedules[timeSlot].end(); ++iter){
        if (*iter != NULL_SCHEDULE){
            int in = iter - schedules[timeSlot].begin(), out = *iter;
            ++inputAvailability[in];
            ++outputAvailability[out];
            // reset match bitset
            inputMatchBitmap[in].reset(timeSlot);
            outputMatchBitmap[out].reset(timeSlot);
        }
    }

    // clear the schedule
    std::fill(schedules[timeSlot].begin(), schedules[timeSlot].end(), NULL_SCHEDULE);
}

void Sliding_Window_QPS::handlePacketDepartures(
	const std::vector<std::pair<int, int>>& virtualDepartures){
	for (const auto& srcDestPair : virtualDepartures){
		assert(inputQueues[srcDestPair.first].get_packet_number(srcDestPair.second) > 0);
		inputQueues[srcDestPair.first].remove_item(srcDestPair.second);
	}
}

void Sliding_Window_QPS::resetBitmap(){
	for (auto& mf : inputMatchBitmap){
	    mf.reset();
    }
	for (auto& mf : outputMatchBitmap){
		mf.reset();
    }
	std::fill(inputAvailability.begin(), inputAvailability.end(), 0);
	std::fill(outputAvailability.begin(), outputAvailability.end(), 0);
}

void Sliding_Window_QPS::reset(){
	BatchScheduler::reset();
	resetBitmap();
	std::fill(inputAvailability.begin(), inputAvailability.end(), batchSize);
    std::fill(outputAvailability.begin(), outputAvailability.end(), batchSize);
	for (size_t i = 0; i < _num_inputs; ++i){
		inputQueues[i].reset();
    }
	_cf_rel_time = 0;
	for (auto& sched : schedules){
        std::fill(sched.begin(), sched.end(), NULL_SCHEDULE);
	}
}

void Sliding_Window_QPS::display(std::ostream& os) const {
	BatchScheduler::display(os);
	os << "---------------------------------------------------------------------\n";
	os << "seed: " << seed << "\n input queues\n";
	for (const auto & item : inputQueues){
		os << item << "\n";
	}
		
}
///< End of implementation for class Sliding_Window_QPS

/**
 * Start of implementation for class SW_QPS_Best_Fit
 */
void SW_QPS_Best_Fit::qps(const saber::IQSwitch* iqSwitch, size_t timeSlot){
    assert(isBatchSizeFixed);

    // handle arrivals
    handlePacketArrivals(iqSwitch);

    _debug_stream << "This is best fit scheduler.\n";
    _debug_stream << "queue state\n" << std::setw(11) << " ";
    for (int f=0; f<_num_inputs; ++f){
        _debug_stream << std::setw(3) << f;
    }
    _debug_stream << "\n";
    for (size_t in=0; in<_num_inputs; ++in){
        _debug_stream << "input " << std::setw(3) << in << ": " << inputQueues[in] << "\n";
    }

    // proposals recieved by each output port
    std::vector<std::vector<int>> out_accepts(num_outputs());

    // shuffle inputs
    std::vector<int> inputs(_num_inputs, 0);
    for (int i = 0; i < _num_inputs; ++i){
        inputs[i] = i;
    }
    std::shuffle(inputs.begin(), inputs.end(), eng);

    // Step 1: Proposing
    for (int i = 0; i < _num_inputs; ++i){
        int in = inputs[i];
        if (getQueueLengthForInput(in) == 0){
            continue; // no packets
        }

        int out = inputQueues[in].sample(eng, Queue_Sampler::QPS); // sampling an output for this input
        assert(inputQueues[in].get_packet_number(out) > 0);
        out_accepts[out].push_back(in);
    }

    std::vector<std::pair<int, int>> virtualDepartures; // virtual departures
    // matching for output ports
    std::vector<int> out_match(num_outputs(), NULL_SCHEDULE);
    // Step 2: Accept
    _debug_stream << "accepting\n";
    for (int out = 0; out < _num_outputs; ++out){
        if (out_accepts[out].empty()){  // no proposal received
            continue;
        }

        //first, compute initial matching
        std::vector<int> proposals = out_accepts[out];
        std::vector<int> initial_match(_num_inputs, NULL_SCHEDULE); // initial match in-f
        std::sort(proposals.begin(), proposals.end(), [=](const int in1, const int in2){
            return inputQueues[in1].get_packet_number(out) > inputQueues[in2].get_packet_number(out);
        });
        int out_avail = outputAvailability[out];
        std::vector<int> input_avail = inputAvailability;
        bitmap_t out_flags = outputMatchBitmap[out];
        std::vector<bitmap_t> in_flags = inputMatchBitmap;
        for (int in: proposals){
            if (out_avail == 0){
                break;
            }
            if (input_avail[in] == 0){
                continue; // no available ts
            }
            // available only when both available
            auto mf = (in_flags[in] | out_flags);
            bool success = false;
            for (int f : saber::Circular_Generator{(int)timeSlot, (int)batchSize, batching}){
                if (!mf.test(f)){
                    in_flags[in].set(f);
                    out_flags.set(f);
                    --input_avail[in];
                    --out_avail;
                    initial_match[in] = f;
                    break; // pay special attention (first fit)
                }
            }
        }

        lemon::ListBpGraph avail_graph;
        typedef lemon::ListBpGraph::RedNode RedNode;
        typedef lemon::ListBpGraph::BlueNode BlueNode;
        lemon::ListBpGraph::NodeMap<int> node_map(avail_graph);
        std::vector <RedNode> red_nodes(batchSize); // frame to red nodes
        std::vector <BlueNode> blue_nodes;


        // add a red node for each available output frame
        for (int f = 0; f < batchSize; ++f){
            if (!outputMatchBitmap[out].test(f)){
                RedNode rn = avail_graph.addRedNode();
                node_map[rn] = f;
                red_nodes[f] = rn;
            }
        }

        // add a blue node for each proposal sender port
        for (int in: out_accepts[out]){
            BlueNode bn = avail_graph.addBlueNode();
            node_map[bn] = in;
            blue_nodes.push_back(bn);
        }

        lemon::ListBpGraph::EdgeMap<bool> initial_matching_map(avail_graph);

        for (auto bn:blue_nodes){
            int in = node_map[bn];
            // add an edge between in-f if both in and out are available at f
            for (int f = 0; f < batchSize; ++f){
                if (!inputMatchBitmap[in].test(f) && !outputMatchBitmap[out].test(f)){
                    lemon::ListBpGraph::Edge edge = avail_graph.addEdge(red_nodes[f], bn);
                    initial_matching_map[edge] = f == initial_match[in];
                }
            }
        }

        lemon::MaxMatching <lemon::ListBpGraph> max_matching(avail_graph);
        max_matching.matchingInit(initial_matching_map);
        max_matching.run();   // compute maximum matching
        for (BlueNode bn:blue_nodes){
            lemon::ListBpGraphBase::Node mate = max_matching.mate(bn);
            if (mate == lemon::INVALID){ continue; }
            int in = node_map[bn], f = node_map[mate];
            // for each pair of matched in-f nodes
            inputMatchBitmap[in].set(f);
            outputMatchBitmap[out].set(f);
            --inputAvailability[in];
            --outputAvailability[out];
            assert(schedules[f][in] == NULL_SCHEDULE);
            schedules[f][in] = out;
            virtualDepartures.emplace_back(in, out);
        }
    }

    handlePacketDepartures(virtualDepartures);
}
///< End of implementation for class SW_QPS_Best_Fit

}///< namespace saber
