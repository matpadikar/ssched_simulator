#include <iostream>
#include "../src/timer.hpp"

using namespace std;

int main(){
    StopWatch wat;
    int a, b;
    cin >> a;
    wat.start();
    b = a + 39;
    wat.stop();
    cout << wat.peek() << endl;
    cout << b << endl;
    return 0;
 }