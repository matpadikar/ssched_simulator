
#ifndef AVAILMAP_HPP
#define AVAILMAP_HPP

#include <vector>
#include <iostream>
#include "utilities.h"

namespace saber {
    // bitmap 63, 62, ..., 0, 127, 126, ... 64, ...
    // 1 means available
    template<int size>
    class AvailabilityMap{
        static_assert(size > 0 && size % 64 == 0);
        uint64_t map[size / 64];
        const int asize;

        public:
        AvailabilityMap(): asize(size/64) {
            reset();
        }

        AvailabilityMap(int rsize): asize(rsize/64 + ((rsize % 64)? 1: 0)) {
            reset();
        }

        void reset(){
            for(int i = 0; i < asize; ++i){
                map[i] = static_cast<u_int64_t>(-1); // 0xffffffffffffffffull
            }
        }

        void set(int pos){
            assert(pos >= 0 && pos < size);
            int apos = pos / 64, bpos = pos % 64;
            map[apos] &= ~(1ull << bpos);
        }

        uint64_t get(int apos){
            return map[apos];
        }

        int ffs(){
            int result = 0, i;
            for(i = 0; result == 0 && i < asize; ++i){
                result = __builtin_ffsll(map[i]);
            }
            //for(int j = 0; j < i; ++j){
            //    if (map[j] != 0) 
            //    print_one_line(std::cout, map[j], j);
            //}

            //result = result + 64 * i - 1;
            //if (result >= rsize) result = -1
            return (result==0)? -1: result + 64 * (i - 1) - 1;
        }

        AvailabilityMap operator&(const AvailabilityMap& other){
            int rasize = std::max(asize, other.asize);
            AvailabilityMap result(rasize * 64);
            for(int i = 0; i < rasize; ++i){
                result.map[i] = map[i] & other.map[i];
            }
            return result;
        }
    };
}

#endif