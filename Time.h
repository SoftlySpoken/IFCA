#ifndef TIME_H
#define TIME_H
#include <unistd.h>
#include <sys/time.h>
//#define TIMES_PER_SEC 2300000000.0  //local CPU 2.3Ghz
//#define TIMES_PER_SEC 2200000000.0  //111服务器 CPU 2.2GHZ
#define TIMES_PER_SEC 2100000000.0  //27服务器&&135服务器 CPU 2.1GHZ




// struct timeval t_start,t_end; 
// double timeCost;
// gettimeofday(&t_start, NULL); 

// gettimeofday(&t_end, NULL); 
// timeCost = t_end.tv_sec - t_start.tv_sec + (t_end.tv_usec - t_start.tv_usec)/1000000.0;
// cout<<"cost: "<<timeCost<<" s"<<endl;


uint64_t rdtsc(void)
{
    unsigned a, d;
    //asm("cpuid");
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return (((uint64_t)a) | (((uint64_t)d) << 32));
}


    // uint64_t rdtscStart=rdtsc();
    // uint64_t rdtscStop=rdtsc();
    // cout << "cost" << (rdtscStop-rdtscStart)/TIMES_PER_SEC << "s" << endl;

#endif