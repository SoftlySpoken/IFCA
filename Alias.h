#ifndef ALIAS_H
#define ALIAS_H

// #include <random>
#include "Random.h"



using namespace std;

class Alias{
public:
	double* p;
	int* h;
	int n;
    int* map;

	Alias(int* rfNode, int rfNodeCount , double* r_f, double r_sum){
        
        map = rfNode;
        n = rfNodeCount;
        p = new double[rfNodeCount];
        h = new int[rfNodeCount];
        int *small = new int[rfNodeCount];
        int smalltop = -1;
        int *big = new int[rfNodeCount];
        int bigtop = -1;
        for( int i = 0 ; i < rfNodeCount ; i++ ){
            p[i] = r_f[rfNode[i]]* rfNodeCount/r_sum;
            if(p[i] > 1){
                big[++bigtop] = i;
            }else{
                small[++smalltop] = i;
            }
        }
        while(bigtop!=-1 && smalltop!=-1){
            int bigindex = big[bigtop];
            int smallindex = small[smalltop--];
            p[bigindex] -= (1-p[smallindex]);
            h[smallindex] = bigindex;
            if(p[bigindex] < 1){
                bigtop--;
                small[++smalltop] = bigindex;
            }
        }
        delete[] small;
		delete[] big;

	}

	~Alias(){
		delete[] p;
		delete[] h;
        delete[] map;
	}
	int SampleNode(Flip& R){
		int firstId = R.Gen_0_1() * n;
		int secondId = R.Gen_0_1() < p[firstId] ? map[firstId] : map[h[firstId]];
		return secondId;
	}
};

#endif