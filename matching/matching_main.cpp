#include "utilities.h"
#include "sb_qps.h"
#include "mwm.hpp"
#include "timer.hpp"
#include <fstream>
#include <cstring>

using namespace saber;
using namespace std;

struct RunningResult{
    double avg_weight, time, time_serena, time_qps;
};


RunningResult run_qps_single_thread(SB_QPS_Configuration config, string input_path){
    SB_QPS_Fixed sb_qps(config);

    ifstream fin(input_path);
    int sz, num;
    fin >> num >> sz;
    double sum = 0;
    double tsum = 0, tqps = 0, qps, tserena = 0, serena;

    for (int i = 0; i < num; ++i){
        vector<vector<int>> weights;
        read_matrix(fin, weights, sz);
        StopWatch timer;
        vector<int> qpsm = sb_qps.schedule(weights);
        double time = timer.peek();
        int qps_weight = SB_QPS_Fixed::weight_computation(qpsm, weights);
        //fout << qps_weight << endl;
        //dump_stream << qpsm << endl;
        sum += qps_weight;
        tsum += time;
        //tqps += qps;
        //tserena += serena;
        
        // vector<int> matching = sb_qps.schedule(weights);
        // int qps_weight = sb_qps.matching_weight();
    }
    //cout << "Average Weight: " << (sum/num) <<endl;
    RunningResult rr;
    rr.avg_weight = (double) sum / num;
    rr.time = tsum / num;
    //rr.time_qps = tqps / num;
    //rr.time_serena = tserena / num;
    return rr;
}

RunningResult run_qps_for_all_matrices(SB_QPS_Configuration config, string input_path){
    //SB_QPS_Fixed sb_qps(config);
    ParallelScheduler ps(config);

    ifstream fin(input_path);
    int sz, num;
    fin >> num >> sz;
    double sum = 0;
    double tsum = 0, tqps = 0, qps, tserena = 0, serena;

    for (int i = 0; i < num; ++i){
        vector<vector<int>> weights;
        read_matrix(fin, weights, sz);
        StopWatch timer;
        vector<int> qpsm = ps.compute(weights);
        double time = timer.peek();
        int qps_weight = SB_QPS_Fixed::weight_computation(qpsm, weights);
        //fout << qps_weight << endl;
        //dump_stream << qpsm << endl;
        sum += qps_weight;
        tsum += time;
        //tqps += qps;
        //tserena += serena;
        
        // vector<int> matching = sb_qps.schedule(weights);
        // int qps_weight = sb_qps.matching_weight();
    }
    //cout << "Average Weight: " << (sum/num) <<endl;
    RunningResult rr;
    rr.avg_weight = (double) sum / num;
    rr.time = tsum / num;
    //rr.time_qps = tqps / num;
    //rr.time_serena = tserena / num;
    return rr;
}

RunningResult run_for_all_matrices(string input_path, vector<int> (*scheduler)(const vector<vector<int>>&) ){
    ifstream fin(input_path);
    int sz, num;
    fin >> num >> sz;
    double sum = 0;
    double tsum = 0;

    for (int i = 0; i < num; ++i){
        vector<vector<int>> weights;
        read_matrix(fin, weights, sz);
        StopWatch timer;
        vector<int> matching = scheduler(weights);
        double time = timer.peek();
        int weight = SB_QPS_Fixed::weight_computation(matching, weights);
        sum += weight;
        //cout << time << endl;
        tsum += time;
    }
    //cout << "Average Weight: " << (sum/num) <<endl;
    RunningResult rr;
    rr.avg_weight = (double) sum / num;
    rr.time = tsum / num;
    return rr;
}

int main(int argc, char* argv[]){
    if (argc < 4){
        cout << "Usage: ./match <input> <conf> <output>" << endl;
        exit(0);
    }
    
    ifstream conf_stream;
    bool use_mwm = false;
    if (strcmp("MWM", argv[2]) == 0){
        use_mwm = true;
    } else {
        conf_stream.open(argv[2]);
    }
    SB_QPS_Configuration config(conf_stream);


    ofstream dump_stream;
    if (argc >= 5){
        dump_stream.open(argv[4], std::ios::out);
    }
    ofstream fout(argv[3]);

    string expr_type;
    conf_stream >> expr_type;

    if (use_mwm){
        auto [weight, time, time_serena, time_qps] = run_for_all_matrices(argv[1], MaximumWeightMatching<int>::maximum_weight_matching);
        fout << weight << "\t" << time << endl;
    }
    else if (expr_type == "constant_rounds_per_frame"){
        int ratio;
        vector<int> frame_sizes;
        vector<double> scores, times;;
        conf_stream >> ratio >> frame_sizes;
        
        for (int frame_size : frame_sizes){
            config.frame_size = frame_size;
            config.num_rounds = ratio * frame_size;

            RunningResult rr;
            if (config.num_threads)
                rr = run_qps_for_all_matrices(config, argv[1]);
            else
                rr = run_qps_single_thread(config, argv[1]);
            scores.push_back(rr.avg_weight);
            times.push_back(rr.time);
        }
        fout << frame_sizes << endl;
        fout << scores << endl;
        fout << times << endl;
    } else if (expr_type == "test_frame_group_size"){
        int ratio;
        vector<int> frame_sizes, group_sizes;
        vector<double> scores, times;
        conf_stream >> frame_sizes >> group_sizes;
        
        for (int frame_size : frame_sizes){
            for (int group_size: group_sizes){
                if (group_size < 0) group_size = frame_size;
                config.frame_size = frame_size;
                config.num_rounds = frame_size;
                config.group_size = group_size;
                RunningResult rr = run_qps_for_all_matrices(config, argv[1]);
                scores.push_back(rr.avg_weight);
                times.push_back(rr.time);
                print_one_line(fout, frame_size, group_size, rr.avg_weight, rr.time, rr.time_qps, rr.time_serena);
            }       
        }
    }

}
