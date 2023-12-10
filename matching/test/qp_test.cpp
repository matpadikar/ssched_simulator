#include "qp_sampler.h"
#include "utilities.h"
#include <random>
#include <iostream>
#include "mwm.hpp"

using namespace std;
using namespace saber;

int main(){
    vector<int> weights({1,2,3,4,5,6,7,8,9,10}), count(10, 0);
    Max_Propose qs(weights);
    random_device rd;
    mt19937 rng(rd());

    /*for(int i = 0; i < 60; ++i){
        int sample = qs.sample();
        if (sample >= 0)
            qs.increment(sample, -1);
        cout << sample << " ";
    }
    cout << endl;*/
    vector<int> weights2({3,2,4,4,6,6,7,4,9,6}); //51
    vector<vector<int>> mat;
    mat.push_back(weights); //55
    mat.push_back(weights2);
    mat.push_back(weights);
    cout << matrix_sum(mat); //161
    return 0;
}