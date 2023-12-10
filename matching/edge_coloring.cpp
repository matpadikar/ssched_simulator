#include "utilities.h"
#include "double_max.h"
#include "mwm.hpp"
#include <fstream>

using namespace saber;
using namespace std;

int main(int argc, char* argv[]){
    ifstream fin(argv[1]);
    ifstream conf_stream(argv[2]);
    int sz, num;
    
    fin >> num;
    SB_QPS_Configuration config(conf_stream);
    EdgeColorDoubleMax ecolor(config);

    for (int i = 0; i < num; ++i){
        fin >> sz;
        vector<vector<int>> weights;
        read_matrix(fin, weights, sz);
        int max_rowsum = max_row_sum(weights);
        int max_colsum = max_column_sum(weights);
        int rounds = ecolor.schedule(weights);

        cout << max_rowsum << "\t" << max_colsum << "\t" << rounds << endl;
    }
    return 0;
}