//  Copyright 2018 U.C. Berkeley RISE Lab
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef FLUENT_HEAVY_HITTERS_SKETCH_HPP
#define FLUENT_HEAVY_HITTERS_SKETCH_HPP

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>
#include "kvs_types.hpp"

const unsigned large_prime = 4294967311l;

class HeavyHittersSketch {
protected:
    unsigned l, B;
    unsigned running_total;

    int **hash_functions;
    int **sketch_array;

    std::unordered_set<Key> key_set;

    unsigned long hash_key(Key key) {
        unsigned long hash = 5381;
        int char_int = 0;
        for(unsigned i = 0; i < key.length(); i++) {
            char_int = (int)(key.at(i));
            hash = ((hash << 5) + hash) + char_int;
        }

        return hash;
    }

    unsigned location(int i, unsigned long hash) {
        return (unsigned)(((long)hash_functions[i][0]*hash+hash_functions[i][1])%large_prime%B);
    }

    std::unordered_set<Key> get_keys(int val, bool upper) {
        std::unordered_set<Key> keys;

        for(const auto& key_auto: key_set) {
            Key key = (Key)(key_auto);

            unsigned long hash = hash_key(key);

            int mincount = 0;
            unsigned hashed_location = 0;
            for (unsigned i = 0; i < d; i++) {
                hashed_location = location(i, hash);
                if((sketch_array[i][hashed_location] < mincount) or (i == 0)) {
                    mincount = sketch_array[i][hashed_location];
                }
            }

            if(upper) {
                if(mincount > val) {
                    keys.insert(key);
                }
            }else{
                if(mincount < val) {
                    keys.insert(key);
                }
            }
        }

        return keys;
    };

    void construct_hash_sketch() {
        running_total = 0;

        sketch_array = new int *[l];
        for(unsigned i = 0; i < l; i++) {
            sketch_array[i] = new int[B];
            for(unsigned j = 0; j < B; j++) {
                sketch_array[i][j] = 0;
            }
        }

        srand(time(NULL));
        hash_functions = new int *[l];
        for(unsigned i = 0; i < l; i++) {
            hash_functions[i] = new int[2];
            hash_functions[i][0] = int(float(rand())*float(large_prime)/float(RAND_MAX) + 1);
            hash_functions[i][1] = int(float(rand())*float(large_prime)/float(RAND_MAX) + 1);
        }
    }
public:
    HeavyHittersSketch(float epsilon, float gamma) {
        l = (int)(ceil(log(1/gamma)));
        B = (int)(ceil(exp(1)/epsilon));

        construct_hash_sketch();
    };

    HeavyHittersSketch(float epsilon, int n) {
        l = (int)(ceil(exp(1) * log(n)));
        B = (int)(ceil(exp(1)/epsilon));

        construct_hash_sketch();
    }

    void add_key(Key key) {
        unsigned long hash = hash_key(key);

        unsigned hashed_location = 0;
        for (unsigned i = 0; i < d; i++) {
            hashed_location = location(i, hash);
            sketch_array[i][hashed_location] = sketch_array[i][hashed_location] + 1;
        }

        running_total = running_total + 1;
    };

    std::unordered_set<Key> get_keys_percentage(float percentage, bool upper){
        int cutoff = (int)(ceil(percentage * running_total));
        return get_keys(cutoff, upper);
    };

    std::unordered_set<Key> get_keys_value(int hit_count, bool upper) {
        return get_keys(hit_count, upper);
    };

    unsigned total(void) {
        return running_total;
    };
};

#endif //FLUENT_HEAVY_HITTERS_SKETCH_HPP
