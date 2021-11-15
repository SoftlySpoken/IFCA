#ifndef RANDOM_H
#define RANDOM_H

#include <iostream>
#include "SFMT-src-1.5.1/SFMT.h"

class Flip{
public:
	sfmt_t sfmt;
    uint32_t seed;
	Flip(){
        seed = time(0);
        sfmt_init_gen_rand(&sfmt,seed);
	}
    int Gen_Max(){
        return sfmt_genrand_uint32(&sfmt)%RAND_MAX;
    }
    double Gen_0_1(){
        return (double)sfmt_genrand_uint32(&sfmt)/double(RAND_MAX)/2;
    }
};

#endif