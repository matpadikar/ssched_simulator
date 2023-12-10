#include "qp_sampler.h"
#include "utilities.h"
#include <random>
#include <iostream>
#include "mwm.hpp"

using namespace std;
using namespace saber;

int main(){
    vector<int> weights({1,2,3,4,5,6,7,8,9,10}), count(10, 0);
    Queue_Sampler qs(weights);
    random_device rd;
    mt19937 rng(rd());

    for(int i = 0; i < 5500000; ++i){
        int sample = qs.sample(rng);
        ++count[sample];
    }

    cout << count << endl;

    count.assign(10, 0);
    qs.remove(2);
    qs.remove(5);
    qs.remove(8);

    for(int i = 0; i < 3700000; ++i){
        int sample = qs.sample(rng);
        ++count[sample];
    }

    cout << count << endl;

    return 0;
}