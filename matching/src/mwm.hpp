#include <lemon/list_graph.h>
#include <lemon/matching.h>
#include <vector>
#include <cassert>
//#include <sb_qps.h>

// A wrapper of Maximum Weight Matching computation using lemon
namespace saber{
template <typename WeightType>
class MaximumWeightMatching{
    typedef lemon::ListGraph Graph;
    typedef Graph::Node Node;
    typedef Graph::Edge Edge;
    typedef Graph::EdgeMap<WeightType> WeightMap; 
    typedef lemon::MaxWeightedMatching<Graph, WeightMap> MaxWeightedMatching;
public:
    static const int NULL_SCHEDULE = -1;
    static std::vector<WeightType> maximum_weight_matching(
        const std::vector<std::vector<WeightType>>& weights){
        std::vector<Node> _left;             
        std::vector<Node> _right;
        
        std::vector<std::vector<WeightType>> _edges; 
        Graph _g;
        WeightMap _weight(_g);


        int num_nodes = weights.size();
        std::vector<int> _in_match(num_nodes, NULL_SCHEDULE);

        for (int i = 0; i < num_nodes; ++i)
            assert(weights[i].size() == num_nodes);

        for (int u = 0; u < num_nodes; ++u)
            _left.push_back(_g.addNode());
        for (int v = 0; v < num_nodes; ++v)
            _right.push_back(_g.addNode());

        _edges.resize(static_cast<unsigned long>(num_nodes),
                std::vector<WeightType>(num_nodes, NULL_SCHEDULE));
        for (int i = 0; i < num_nodes; ++i) {
            for (int j = 0; j < num_nodes; ++j) {
                Edge e = _g.addEdge(_left[i], _right[j]);
                _weight[e] = weights[i][j];
                _edges[i][j] = _g.id(e);
            }
        }

        MaxWeightedMatching mwm(_g, _weight);
        mwm.run();

        Node u, v;
        /* read results */
        for (int i = 0; i < num_nodes; ++i) {
            u = _left[i];
            v = mwm.mate(u);
            if (v == lemon::INVALID)
            _in_match[i] = -1; /* make sure every i is setting */
            else {
            assert(_g.id(v) >= num_nodes &&
                    "Any input's mate should be an output!");
            _in_match[i] = _g.id(v) - num_nodes;
            }
        }

        return _in_match;
    }
};
}