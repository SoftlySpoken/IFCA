#ifndef LIB_H
#define LIB_H

#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <math.h>
#include <cmath>
// #include <limits.h>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <thread>
#include <string>
#include "Alias.h"
#include "Time.h"
// #include <sys/time.h>    // For gettimeofday
#include <time.h>   // For clock_gettime
#include <limits>
#include <random>

using namespace std;


bool maxScoreCmp(const pair<int, double>& a, const pair<int, double>& b){
    return a.second > b.second;
}

bool maxLevelCmp(const pair<int, int>& a, const pair<int, int>& b){
    return a.second > b.second;
}




class pqcompare
{
    bool reverse;
    public:
    pqcompare(const bool& revparam=false){reverse=revparam;}
    bool operator() (const pair<int, double>& lhs, const pair<int, double>&rhs) const
    {
        if (reverse) return (lhs.second > rhs.second);
        else return (lhs.second < rhs.second);
    }
};

void get_pi_TopK(vector<double> pi , int k){
    priority_queue<pair<int,double> , vector< pair<int,double> > , pqcompare> topKlist(pqcompare(true));
    for(int sloop = 0 ; sloop < k ; sloop++ ){
        topKlist.push(make_pair( sloop , pi[sloop] ));
    }
    for(int sloop = k ; sloop < pi.size() ; sloop++ ){
        if( topKlist.top().second < pi[sloop] ){
            topKlist.pop();
            topKlist.push(make_pair( sloop , pi[sloop] ));
        }
    }
    vector<pair<int, double> > res;
    for( int pr = 0 ; pr < k ; pr++ ){
        res.push_back(topKlist.top());
        topKlist.pop();
    }
    for( int pr = k-1 ; pr >= 0 ; pr-- ){
        cout<<res[pr].first<<" ";
        cout<<res[pr].second<<endl;
    }
}

double maxError(vector<double> s_real, double* pi , int k ){
    double err = 0.0;
    for( int mrl = 0 ; mrl < k ; mrl++ ){
        double tmpErr = abs(pi[mrl] - s_real[mrl]);
        if( tmpErr > err ){
            err = tmpErr;
        }
    }
    return err;
}


double maxRelativeError(vector<double> s_real, double* pi , int n){
    double err = 0.0;
    for( int mrl = 0 ; mrl < n ; mrl++ ){
        if(s_real[mrl]<(double)1/n) continue;
        double tmpErr = abs(pi[mrl] - s_real[mrl])/s_real[mrl];
        if( tmpErr > err ){
            err = tmpErr;
        }
    }
    return err;
}



double maxNormalizedError(vector<double> s_real, double* pi , int k , int* outDeg){
    double err = 0.0;
    for( int mrl = 0 ; mrl < k ; mrl++ ){
        double tmpErr = abs(pi[mrl] - s_real[mrl])/outDeg[mrl];
        if( tmpErr > err ){
            err = tmpErr;
        }
    }
    return err;
}


unordered_map<int, double> getRealTopKMap(int s, int k , string filename){
    string gtPath = "groundtruth/"+filename+"/"+to_string(s)+".txt";
    ifstream real(gtPath);
    assert(real.is_open());
    unordered_map<int, double> s_map;
    for(int i = 0; i < k; i++){
        int v; 
        double sim;
        real >> v >> sim;
        s_map[v] = sim;
    }
    real.close();
    return s_map;
}

    vector<double> getReal( int s, string groundtruthPath , int n ){
        string gtPath = groundtruthPath+"/"+to_string(s)+".txt";
        ifstream real(gtPath);
        assert(real.is_open());
        vector<double> s_real(n,0);
        for(int i = 0; i < n; i++){
            int v; 
            double sim;
            real >> v >> sim;
            s_real[v] = sim;
        }
        real.close();
        return s_real;
    }

void split_line(){
    // cout<<"--------------------------"<<endl;
}


#endif
