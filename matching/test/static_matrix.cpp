#include "qp_sampler.h"
#include "utilities.h"
#include <random>
#include <iostream>
#include "mwm.hpp"

using namespace std;
using namespace saber;
#include "traffic_generator/traffic_pattern_factory.h"
#include <fstream>
#include <random>
#include <string>

int main(int argc, char* argv[]){
    ifstream conf(argv[1]);
    std::string output;
    int row_num, row_sum, type, matrix_num;
    conf >> type >> row_num >> row_sum >> matrix_num >> output;
    std::random_device rd;


    TrafficPatternName pattern_name = static_cast<TrafficPatternName>(type);
    auto pattern_ptr = TrafficPatternFactory::Create(pattern_name, row_num, rd()); 

    ofstream fout(output);
    fout << matrix_num << std::endl;
    for (int i = 0; i < matrix_num; ++i){
        auto matrix = static_traffic_matrix(row_sum, row_num, row_num, *pattern_ptr);
        fout << row_num << std::endl;
        fout << matrix;
    }
    

   
    return 0;
}