#ifndef BASE_H
#define BASE_H

#include <string.h>
#include "graph.h"

using namespace std;

class Base: public Graph
{
public:
    vector<double> residue1;
    vector<double> residue2;
    vector<double> reserve1;
    vector<double> reserve2;
    vector<int> l1;
    vector<int> l2;
    vector<int> fSetMark;
    vector<bool> explored1;
    vector<bool> explored2;

    vector<int> nodetosuper1;
    vector<int> nodetosuper2;

    double alpha;
    double theta;
    double start_ratio, step;

    Base(string dPath , string uPath, string qPath , int qSize, double theta_, double alpha_, int num_partitions, double _start_ratio=100, double _step=10): Graph(dPath, uPath, qPath, qSize),
    start_ratio(_start_ratio), step(_step)
    {
        LoadGraph(num_partitions);

        residue1 = vector<double>(n + 1,0);
        residue2 = vector<double>(n + 1,0);
        reserve1 = vector<double>(n + 1,0);
        reserve2 = vector<double>(n + 1,0);
        l1 = vector<int>(n + 1,-1);
        l2 = vector<int>(n + 1,-1);

        alpha = alpha_;
        theta = theta_;
        fSetMark = vector<int>(n + 1,-1);
        explored1 = vector<bool>(n + 1,false);
        explored2 = vector<bool>(n + 1,false);
        nodetosuper1 = vector<int>(n + 1,-1);
        nodetosuper2 = vector<int>(n + 1,-1);
    }

    void InitPara(int sourceNode = 0, int targetNode = 0)
    {
        residue1.assign(n, 0);
        residue2.assign(n, 0);
        l1.assign(n, -1);
        l2.assign(n, -1);
        // fSetMark.assign(n, -1);
        nodetosuper1.assign(n, -1);
        nodetosuper2.assign(n, -1);
        explored1.assign(n, false);
        explored2.assign(n, false);
    }

    int BBFS(int sourceNode, int targetNode)
    {
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        l1[sourceNode] = 0;
        l2[targetNode] = 0;

        while(!candidatePush1.empty() && !candidatePush2.empty())
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                candidatePush1.pop();
                int fwdSz = outDeg[curNode], outNode;
                for (int i = 0; i < fwdSz; i++)
                {
                    outNode = fwdG[curNode][i];
                    if(!explored1[outNode])
                    {
                        candidatePush1_next.push(outNode);
                        explored1[outNode] = true;
                        l1[outNode] = 0;
                        if (l2[outNode] != -1)
                            return 0;
                    }
                }  
            }
            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                candidatePush2.pop();
                int bwdSz = inDeg[curNode], inNode;
                for (int i = 0; i < bwdSz; i++)
                {
                    inNode = bwdG[curNode][i];
                    if(!explored2[inNode])
                    {
                        candidatePush2_next.push(inNode);
                        explored2[inNode] = true;
                        l2[inNode] = 0;
                        if (l1[inNode] != -1)
                            return 0;
                    }
                }
            }
            if(candidatePush1_next.empty() || candidatePush2_next.empty())
                return -1;

            swap(candidatePush1_next , candidatePush1);
            swap(candidatePush2_next , candidatePush2);
        }

        return -1;
    }

    double Push(int sourceNode, double init_rmax)
    {
        struct timespec start_at, end_at;
        
        clock_gettime(CLOCK_MONOTONIC, &start_at);
        queue<int> candidatePush1;
        candidatePush1.push(sourceNode);
        residue1[sourceNode] = 1;
        int nodeNumPush1, unvisitedNumPush1;

        while(!candidatePush1.empty())
        {
            int curNode = candidatePush1.front();
            candidatePush1.pop();

            if( residue1[curNode] / outDeg[curNode] >= init_rmax )
            {
                double baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                for( int adj : fwdG[curNode] )
                {
                    residue1[adj] += baseGain;
                    if(residue1[adj]/outDeg[adj] >= init_rmax )
                        candidatePush1.push(adj);
                }
                residue1[curNode] = 0;
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end_at);
        return timeDiff(start_at, end_at);
    }

    int Push(int sourceNode , int targetNode, double init_rmax)
    {
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        struct timespec start_at, end_at;
        
        double  rmax = 0.01;
        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        residue1[sourceNode] = 1;
        residue2[targetNode] = 1;
        l1[sourceNode] = 0;
        l2[targetNode] = 0;
        int nodeNumPush1, unvisitedNumPush1;
        int nodeNumPush2, unvisitedNumPush2;

        while( rmax >= init_rmax )
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                candidatePush1.pop();

                if( residue1[curNode] / outDeg[curNode] >= rmax )
                {
                    double baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                    for( int adj : fwdG[curNode] )
                    {
                        if (l1[adj] == -1)
                        {
                            l1[adj] = 0;
                            if (l2[adj] != -1)
                                return 0;
                        }
                        residue1[adj] += baseGain;
                        if(residue1[adj]/outDeg[adj] >= rmax )
                            candidatePush1.push(adj);
                        else
                            candidatePush1_next.push(adj);
                    }
                    residue1[curNode] = 0;
                }
                else
                    candidatePush1_next.push(curNode);
            }
            if(candidatePush1_next.empty())
                return -1;

            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                candidatePush2.pop();

                if( residue2[curNode]/inDeg[curNode] >= rmax )
                {
                    double baseGain = (1-alpha)*residue2[curNode]/inDeg[curNode];
                    for( int adj : bwdG[curNode] )
                    {
                        if (l2[adj] == -1)
                        {
                            l2[adj] = 0;
                            if(l1[adj] != -1)
                                return 0;
                        }
                        residue2[adj] += baseGain;
                        if(residue2[adj]/inDeg[adj] >= rmax )
                            candidatePush2.push(adj);
                        else
                            candidatePush2_next.push(adj);
                    }
                    residue2[curNode] = 0;
                }
                else
                    candidatePush2_next.push(curNode);
            }
            if(candidatePush2_next.empty())
                return -1;

            swap(candidatePush1,candidatePush1_next);
            swap(candidatePush2,candidatePush2_next);

            rmax /= 10;
        }

        return -1;
    }

    double BwPush(int sourceNode, double init_rmax)
    {
        struct timespec start_at, end_at;
        
        clock_gettime(CLOCK_MONOTONIC, &start_at);
        queue<int> candidatePush1;
        candidatePush1.push(sourceNode);
        residue1[sourceNode] = 1;

        while(!candidatePush1.empty())
        {
            int curNode = candidatePush1.front();
            candidatePush1.pop();

            if( residue1[curNode] >= init_rmax )
            {
                // double baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                for( int adj : fwdG[curNode] )
                {
                    residue1[adj] += (1-alpha)*residue1[curNode]/inDeg[adj];
                    if(residue1[adj] >= init_rmax)
                        candidatePush1.push(adj);
                }
                residue1[curNode] = 0;
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &end_at);
        return timeDiff(start_at, end_at);
    }

    int BwPush(int sourceNode , int targetNode, double init_rmax)
    {
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        struct timespec start_at, end_at;
        
        double  rmax = 0.01;
        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        residue1[sourceNode] = 1;
        residue2[targetNode] = 1;
        l1[sourceNode] = 0;
        l2[targetNode] = 0;
        int nodeNumPush1, unvisitedNumPush1;
        int nodeNumPush2, unvisitedNumPush2;

        while( rmax >= init_rmax )
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                candidatePush1.pop();

                if( residue1[curNode] >= rmax )
                {
                    // double baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                    for( int adj : fwdG[curNode] )
                    {
                        if (l1[adj] == -1)
                        {
                            l1[adj] = 0;
                            if (l2[adj] != -1)
                                return 0;
                        }
                        residue1[adj] += (1-alpha)*residue1[curNode]/inDeg[adj];
                        if(residue1[adj] >= rmax )
                            candidatePush1.push(adj);
                        else
                            candidatePush1_next.push(adj);
                    }
                    residue1[curNode] = 0;
                }
                else
                    candidatePush1_next.push(curNode);
            }
            if(candidatePush1_next.empty())
                return -1;

            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                candidatePush2.pop();

                if( residue2[curNode] >= rmax )
                {
                    // double baseGain = (1-alpha)*residue2[curNode]/inDeg[curNode];
                    for( int adj : bwdG[curNode] )
                    {
                        if (l2[adj] == -1)
                        {
                            l2[adj] = 0;
                            if(l1[adj] != -1)
                                return 0;
                        }
                        residue2[adj] += (1-alpha)*residue2[curNode]/outDeg[adj];
                        if(residue2[adj] >= rmax )
                            candidatePush2.push(adj);
                        else
                            candidatePush2_next.push(adj);
                    }
                    residue2[curNode] = 0;
                }
                else
                    candidatePush2_next.push(curNode);
            }
            if(candidatePush2_next.empty())
                return -1;

            swap(candidatePush1,candidatePush1_next);
            swap(candidatePush2,candidatePush2_next);

            rmax /= 10;
        }

        return -1;
    }

    int PushContract(int sourceNode, int targetNode, double init_rmax)
    {
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        struct timespec start_at, end_at;
        
        init_rmax = 10000.0 / (48.0 * n * log(m));
        double start_rmax = 0.01;
        if (start_rmax <= init_rmax)
            start_rmax = init_rmax * 10.0;
        double  rmax = start_rmax;
        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        residue1[sourceNode] = 1;
        residue2[targetNode] = 1;
        l1[sourceNode] = 0;
        l2[targetNode] = 0;
        int nodeNumPush1, unvisitedNumPush1;
        int nodeNumPush2, unvisitedNumPush2;

        // Counters for contraction
        int intEdges1 = 0;
        int intEdges2 = 0;
        int mapCnt1 = -2, mapCnt2 = -2;
        int nLeft1 = n - 1, nLeft2 = n - 1, mLeft1 = m, mLeft2 = m; // For the cost model

        while (true)
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                if (!explored1[curNode])
                {
                    intEdges1 += outDeg[curNode];
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                candidatePush1.pop();
                if( residue1[curNode] / outDeg[curNode] >= rmax )
                {
                    int fwdSz = outDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] == -1)
                        {
                            nLeft1--;
                            l1[adj] = 0;
                            if (l2[adj] != -1)
                                return 0;
                        }
                        if (nodetosuper1[adj] < -1 && nodetosuper1[adj] > mapCnt1)
                            mappedNum++;
                    }
                    double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        baseGain = (1-alpha)*residue1[curNode]/(outDeg[curNode] - mappedNum + 1);
                        residue1[n] += baseGain;
                        if (residue1[n] / outDeg[n] >= rmax)
                            candidatePush1.push(n);
                        else
                            candidatePush1_next.push(n);
                    }
                    else
                        baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = fwdG[curNode][i];
                        if (!(nodetosuper1[adj] < -1 && nodetosuper1[adj] > mapCnt1))
                        {
                            residue1[adj] += baseGain;
                            if (residue1[adj] / outDeg[adj] >= rmax)
                            candidatePush1.push(adj);
                        else
                            candidatePush1_next.push(adj);
                        }
                    }
                    residue1[curNode] = 0;
                }
                else
                    candidatePush1_next.push(curNode);
            }
            if(candidatePush1_next.empty())
                return -1;
            // Perform contraction if the condition is met
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh1;
                while (!candidatePush1_next.empty())
                {
                    int curNode = candidatePush1_next.front();
                    candidatePush1_next.pop();
                    int fwdSz = outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] != 0 && superNeigh1.find(adj) == superNeigh1.end())
                        {
                            superNeigh1.insert(adj);
                            fwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                outDeg[n] = superDeg;

                // Clear counters
                mLeft1 -= intEdges1;
                intEdges1 = 0;

                // Re-initialize
                candidatePush1.push(n);
                residue1[n] = 1;
                l1[n] = 0;
                explored1[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt1--;
            }
            else
                swap(candidatePush1, candidatePush1_next);

            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                if (!explored2[curNode])
                {
                    intEdges2 += inDeg[curNode];
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                candidatePush2.pop();
                if( residue2[curNode] / inDeg[curNode] >= rmax )
                {
                    int bwdSz = inDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] == -1)
                        {
                            nLeft2--;
                            l2[adj] = 0;
                            if (l1[adj] != -1)
                                return 0;
                        }
                        if (nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2)  // Mapped to supernode
                            mappedNum++;
                    }
                    double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        baseGain = (1-alpha)*residue2[curNode]/(inDeg[curNode] - mappedNum + 1);
                        residue2[n] += baseGain;
                        if (residue2[n] / inDeg[n] >= rmax)
                            candidatePush2.push(n);
                        else
                            candidatePush2_next.push(n);
                    }
                    else
                        baseGain = (1-alpha)*residue2[curNode]/inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = bwdG[curNode][i];
                        if (!(nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2))
                        {
                            residue2[adj] += baseGain;
                            if (residue2[adj] / inDeg[adj] >= rmax)
                                candidatePush2.push(adj);
                            else
                                candidatePush2_next.push(adj);
                        }
                    }
                    residue2[curNode] = 0;
                }
                else
                    candidatePush2_next.push(curNode);
            }
            if(candidatePush2_next.empty())
                return -1;
            
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh2;
                while (!candidatePush2_next.empty())
                {
                    int curNode = candidatePush2_next.front();
                    candidatePush2_next.pop();
                    int bwdSz = inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] != 0 && superNeigh2.find(adj) == superNeigh2.end())
                        {
                            superNeigh2.insert(adj);
                            bwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                inDeg[n] = superDeg;

                // Clear counters
                mLeft2 -= intEdges2;
                intEdges2 = 0;

                // Re-initialize
                candidatePush2.push(n);
                residue2[n] = 1;
                l2[n] = 0;
                explored2[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt2--;
            }
            else
                swap(candidatePush2,candidatePush2_next);

            // candidatePush1_next, candidatePush2_next are empty
            // All contracted vertices are marked as explored

            rmax /= 10;
        }

        return -1;
    }

    int BwPushContract(int sourceNode, int targetNode, double init_rmax)
    {
        // cout << "BwPushContract" << endl;
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        struct timespec start_at, end_at;
        
        // init_rmax = 10000.0 / (48.0 * n * log(m));
        double start_rmax = 100;
        init_rmax = 10;
        if (start_rmax <= init_rmax)
            start_rmax = init_rmax * 10.0;
        double  rmax = start_rmax;
        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        residue1[sourceNode] = 1;
        residue2[targetNode] = 1;
        l1[sourceNode] = 0;
        l2[targetNode] = 0;
        int nodeNumPush1, unvisitedNumPush1;
        int nodeNumPush2, unvisitedNumPush2;

        // Counters for contraction
        int intEdges1 = 0;
        int intEdges2 = 0;
        int mapCnt1 = -2, mapCnt2 = -2;
        int nLeft1 = n - 1, nLeft2 = n - 1, mLeft1 = m, mLeft2 = m; // For the cost model

        while (true)
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                if (!explored1[curNode])
                {
                    intEdges1 += outDeg[curNode];
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                candidatePush1.pop();
                if( residue1[curNode] >= rmax )
                {
                    int fwdSz = outDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] == -1)
                        {
                            nLeft1--;
                            l1[adj] = 0;
                            if (l2[adj] != -1)
                                return 0;
                        }
                        if (nodetosuper1[adj] < -1 && nodetosuper1[adj] > mapCnt1)
                            mappedNum++;
                    }
                    // double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        // baseGain = (1-alpha)*residue1[curNode]/(outDeg[curNode] - mappedNum + 1);
                        residue1[n] += (1-alpha) * residue1[curNode] * mappedNum / inDeg[n];
                        if (residue1[n] / outDeg[n] >= rmax)
                            candidatePush1.push(n);
                        else
                            candidatePush1_next.push(n);
                    }
                    // else
                    //     baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = fwdG[curNode][i];
                        if (!(nodetosuper1[adj] < -1 && nodetosuper1[adj] > mapCnt1))
                        {
                            residue1[adj] += (1-alpha) * residue1[curNode] / inDeg[adj];
                            candidatePush1.size();  // For gdb
                            if (residue1[adj] >= rmax)
                                candidatePush1.push(adj);
                        else
                            candidatePush1_next.push(adj);
                        }
                    }
                    residue1[curNode] = 0;
                }
                else
                    candidatePush1_next.push(curNode);
            }
            if(candidatePush1_next.empty())
                return -1;
            // Perform contraction if the condition is met
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh1;
                while (!candidatePush1_next.empty())
                {
                    int curNode = candidatePush1_next.front();
                    candidatePush1_next.pop();
                    int fwdSz = outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] != 0 && superNeigh1.find(adj) == superNeigh1.end())
                        {
                            superNeigh1.insert(adj);
                            fwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                outDeg[n] = superDeg;

                // Clear counters
                mLeft1 -= intEdges1;
                intEdges1 = 0;

                // Re-initialize
                candidatePush1.push(n);
                residue1[n] = 1;
                l1[n] = 0;
                explored1[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt1--;
            }
            else
                swap(candidatePush1, candidatePush1_next);

            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                if (!explored2[curNode])
                {
                    intEdges2 += inDeg[curNode];
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                candidatePush2.pop();
                if( residue2[curNode] >= rmax )
                {
                    int bwdSz = inDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] == -1)
                        {
                            nLeft2--;
                            l2[adj] = 0;
                            if (l1[adj] != -1)
                                return 0;
                        }
                        if (nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2)  // Mapped to supernode
                            mappedNum++;
                    }
                    // double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        // baseGain = (1-alpha)*residue2[curNode]/(inDeg[curNode] - mappedNum + 1);
                        residue2[n] += (1-alpha)*residue2[curNode] * mappedNum / outDeg[n];
                        if (residue2[n]  >= rmax)
                            candidatePush2.push(n);
                        else
                            candidatePush2_next.push(n);
                    }
                    // else
                    //     baseGain = (1-alpha)*residue2[curNode]/inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = bwdG[curNode][i];
                        if (!(nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2))
                        {
                            residue2[adj] += (1-alpha)*residue2[curNode] / outDeg[adj];
                            if (residue2[adj] >= rmax)
                                candidatePush2.push(adj);
                            else
                                candidatePush2_next.push(adj);
                        }
                    }
                    residue2[curNode] = 0;
                }
                else
                    candidatePush2_next.push(curNode);
            }
            if(candidatePush2_next.empty())
                return -1;
            
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh2;
                while (!candidatePush2_next.empty())
                {
                    int curNode = candidatePush2_next.front();
                    candidatePush2_next.pop();
                    int bwdSz = inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] != 0 && superNeigh2.find(adj) == superNeigh2.end())
                        {
                            superNeigh2.insert(adj);
                            bwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                inDeg[n] = superDeg;

                // Clear counters
                mLeft2 -= intEdges2;
                intEdges2 = 0;

                // Re-initialize
                candidatePush2.push(n);
                residue2[n] = 1;
                l2[n] = 0;
                explored2[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt2--;
            }
            else
                swap(candidatePush2,candidatePush2_next);

            // candidatePush1_next, candidatePush2_next are empty
            // All contracted vertices are marked as explored

            rmax /= 10;
        }

        return -1;
    }

    int PushContractTraversal(int sourceNode, int targetNode, double init_rmax, int rounds=-1)
    {
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        struct timespec start_at, end_at;
        
        // init_rmax = 10000.0 / (48.0 * n * log(m));
        // double start_rmax = 0.01;
        double start_rmax = init_rmax * 100.0;
        // if (start_rmax <= init_rmax)
        //     start_rmax = init_rmax * 10.0;
        double  rmax = start_rmax;
        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        residue1[sourceNode] = 1;
        residue2[targetNode] = 1;
        l1[sourceNode] = 0;
        l2[targetNode] = 0;
        int nodeNumPush1, unvisitedNumPush1;
        int nodeNumPush2, unvisitedNumPush2;

        // Counters for contraction
        int intEdges1 = 0;
        int intEdges2 = 0;
        int mapCnt1 = -2, mapCnt2 = -2;
        int nLeft1 = n - 1, nLeft2 = n - 1, mLeft1 = m, mLeft2 = m; // For the cost model

        double cost_push, cost_bbfs, lambda = 1.76, coeff = 1.0 / (log(n) + 0.58);
        int curRounds = 0;
        while (true)
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                if (!explored1[curNode])
                {
                    intEdges1 += outDeg[curNode];
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                candidatePush1.pop();
                if( residue1[curNode] / outDeg[curNode] >= rmax )
                {
                    int fwdSz = outDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] == -1)
                        {
                            nLeft1--;
                            l1[adj] = 0;
                            if (l2[adj] != -1)
                                return 0;
                        }
                    }
                    double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        baseGain = (1-alpha)*residue1[curNode]/(outDeg[curNode] - mappedNum + 1);
                        residue1[n] += baseGain;
                        if (residue1[n] / outDeg[n] >= rmax)
                            candidatePush1.push(n);
                        else
                            candidatePush1_next.push(n);
                    }
                    else
                        baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = fwdG[curNode][i];
                        if (!(nodetosuper1[adj] < -1 && nodetosuper1[adj] > mapCnt1))
                        {
                            residue1[adj] += baseGain;
                            if (residue1[adj] / outDeg[adj] >= rmax)
                            candidatePush1.push(adj);
                        else
                            candidatePush1_next.push(adj);
                        }
                    }
                    residue1[curNode] = 0;
                }
                else
                    candidatePush1_next.push(curNode);
            }
            if(candidatePush1_next.empty())
                return -1;
            
            // Perform contraction if the condition is met
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh1;
                while (!candidatePush1_next.empty())
                {
                    int curNode = candidatePush1_next.front();
                    candidatePush1_next.pop();
                    int fwdSz = outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] != 0 && superNeigh1.find(adj) == superNeigh1.end())
                        {
                            superNeigh1.insert(adj);
                            fwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                outDeg[n] = superDeg;

                // Clear counters
                mLeft1 -= intEdges1;
                intEdges1 = 0;

                // Re-initialize
                candidatePush1.push(n);
                residue1[n] = 1;
                l1[n] = 0;
                explored1[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt1--;
            }
            else
                swap(candidatePush1, candidatePush1_next);

            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                if (!explored2[curNode])
                {
                    intEdges2 += inDeg[curNode];
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                candidatePush2.pop();
                if( residue2[curNode] / inDeg[curNode] >= rmax )
                {
                    int bwdSz = inDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] == -1)
                        {
                            nLeft2--;
                            l2[adj] = 0;
                            if (l1[adj] != -1)
                                return 0;
                        }

                        if (nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2)  // Mapped to supernode
                            mappedNum++;
                    }
                    double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        baseGain = (1-alpha)*residue2[curNode]/(inDeg[curNode] - mappedNum + 1);
                        residue2[n] += baseGain;
                        if (residue2[n] / inDeg[n] >= rmax)
                            candidatePush2.push(n);
                        else
                            candidatePush2_next.push(n);
                    }
                    else
                        baseGain = (1-alpha)*residue2[curNode]/inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = bwdG[curNode][i];
                        if (!(nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2))
                        {
                            residue2[adj] += baseGain;
                            if (residue2[adj] / inDeg[adj] >= rmax)
                                candidatePush2.push(adj);
                            else
                                candidatePush2_next.push(adj);
                        }
                    }
                    residue2[curNode] = 0;
                }
                else
                {
                    candidatePush2_next.push(curNode);
                }
            }
            if(candidatePush2_next.empty())
                return -1;
            
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh2;
                while (!candidatePush2_next.empty())
                {
                    int curNode = candidatePush2_next.front();
                    candidatePush2_next.pop();
                    int bwdSz = inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] != 0 && superNeigh2.find(adj) == superNeigh2.end())
                        {
                            superNeigh2.insert(adj);
                            bwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                inDeg[n] = superDeg;

                // Clear counters
                mLeft2 -= intEdges2;
                intEdges2 = 0;

                // Re-initialize
                candidatePush2.push(n);
                residue2[n] = 1;
                l2[n] = 0;
                explored2[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt2--;
            }
            else
                swap(candidatePush2,candidatePush2_next);

            // candidatePush1_next, candidatePush2_next are empty
            // All contracted vertices are marked as explored

            // Cost model for switching to BBFS (break).
            cost_push = 2 * lambda * (1.0 / (alpha * init_rmax) - 1.0 / (alpha * rmax)) \
                + lambda * (nLeft1 + nLeft2) / (coeff / (alpha * (1 - alpha) * init_rmax)) * (1.0 / (alpha * init_rmax) - 1.0 / (alpha * start_rmax));
            mLeft1 -= intEdges1;
            mLeft2 -= intEdges2;
            cost_bbfs = (nLeft1 + mLeft1) + (nLeft2 + mLeft2);
            curRounds++;
            if (cost_push > cost_bbfs || curRounds == rounds)
                break;

            rmax /= 10;
        }

        // BBFS
        while(!candidatePush1.empty() && !candidatePush2.empty())
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                candidatePush1.pop();
                int fwdSz = outDeg[curNode], outNode;
                for (int i = 0; i < fwdSz; i++)
                {
                    outNode = fwdG[curNode][i];
                    if(!explored1[outNode])
                    {
                        candidatePush1_next.push(outNode);
                        explored1[outNode] = true;
                        l1[outNode] = 0;
                        if (l2[outNode] != -1)
                            return 0;
                    }
                }  
            }
            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                candidatePush2.pop();
                int bwdSz = inDeg[curNode], inNode;
                for (int i = 0; i < bwdSz; i++)
                {
                    inNode = bwdG[curNode][i];
                    if(!explored2[inNode])
                    {
                        candidatePush2_next.push(inNode);
                        explored2[inNode] = true;
                        l2[inNode] = 0;
                        if (l1[inNode] != -1)
                            return 0;
                    }
                }
            }
            if(candidatePush1_next.empty() || candidatePush2_next.empty())
                return -1;

            swap(candidatePush1_next , candidatePush1);
            swap(candidatePush2_next , candidatePush2);
        }

        return -1;
    }

    int BwPushContractTraversal(int sourceNode, int targetNode, double init_rmax)
    {
        if( sourceNode == targetNode ) return 0;
        if( outDeg[sourceNode] == 0 || inDeg[targetNode] == 0 ) return -1;

        struct timespec start_at, end_at;
        
        // init_rmax = 10000.0 / (48.0 * n * log(m));
        // double start_rmax = 0.01;
        // double start_rmax = 100;
        double start_rmax = init_rmax * 100;
        // init_rmax = 10;
        // if (start_rmax <= init_rmax)
        //     start_rmax = init_rmax * 10.0;
        double  rmax = start_rmax;
        queue<int> candidatePush1;
        queue<int> candidatePush1_next;
        queue<int> candidatePush2;
        queue<int> candidatePush2_next;
        candidatePush1.push(sourceNode);
        candidatePush2.push(targetNode);
        residue1[sourceNode] = 1;
        residue2[targetNode] = 1;
        l1[sourceNode] = 0;
        l2[targetNode] = 0;
        int nodeNumPush1, unvisitedNumPush1;
        int nodeNumPush2, unvisitedNumPush2;

        // Counters for contraction
        int intEdges1 = 0;
        int intEdges2 = 0;
        int mapCnt1 = -2, mapCnt2 = -2;
        int nLeft1 = n - 1, nLeft2 = n - 1, mLeft1 = m, mLeft2 = m; // For the cost model

        double cost_push, cost_bbfs, lambda = 1.76, coeff = 1.0 / (log(n) + 0.58);
        while (true)
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                if (!explored1[curNode])
                {
                    intEdges1 += outDeg[curNode];
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                candidatePush1.pop();
                if( residue1[curNode] / outDeg[curNode] >= rmax )
                {
                    int fwdSz = outDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] == -1)
                        {
                            nLeft1--;
                            l1[adj] = 0;
                            if (l2[adj] != -1)
                                return 0;
                        }
                    }
                    double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        baseGain = (1-alpha)*residue1[curNode]/(outDeg[curNode] - mappedNum + 1);
                        residue1[n] += baseGain;
                        if (residue1[n] / outDeg[n] >= rmax)
                            candidatePush1.push(n);
                        else
                            candidatePush1_next.push(n);
                    }
                    else
                        baseGain = (1-alpha)*residue1[curNode]/outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = fwdG[curNode][i];
                        if (!(nodetosuper1[adj] < -1 && nodetosuper1[adj] > mapCnt1))
                        {
                            residue1[adj] += baseGain;
                            if (residue1[adj] / outDeg[adj] >= rmax)
                            candidatePush1.push(adj);
                        else
                            candidatePush1_next.push(adj);
                        }
                    }
                    residue1[curNode] = 0;
                }
                else
                    candidatePush1_next.push(curNode);
            }
            if(candidatePush1_next.empty())
                return -1;
            
            // Perform contraction if the condition is met
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh1;
                while (!candidatePush1_next.empty())
                {
                    int curNode = candidatePush1_next.front();
                    candidatePush1_next.pop();
                    int fwdSz = outDeg[curNode];
                    for (int i = 0; i < fwdSz; i++)
                    {
                        int adj = fwdG[curNode][i];
                        if (l1[adj] != 0 && superNeigh1.find(adj) == superNeigh1.end())
                        {
                            superNeigh1.insert(adj);
                            fwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored1[curNode] = true;
                    nodetosuper1[curNode] = mapCnt1;
                }
                outDeg[n] = superDeg;

                // Clear counters
                mLeft1 -= intEdges1;
                intEdges1 = 0;

                // Re-initialize
                candidatePush1.push(n);
                residue1[n] = 1;
                l1[n] = 0;
                explored1[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt1--;
            }
            else
                swap(candidatePush1, candidatePush1_next);

            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                if (!explored2[curNode])
                {
                    intEdges2 += inDeg[curNode];
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                candidatePush2.pop();
                if( residue2[curNode] / inDeg[curNode] >= rmax )
                {
                    int bwdSz = inDeg[curNode];
                    int mappedNum = 0;
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] == -1)
                        {
                            nLeft2--;
                            l2[adj] = 0;
                            if (l1[adj] != -1)
                                return 0;
                        }

                        if (nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2)  // Mapped to supernode
                            mappedNum++;
                    }
                    double baseGain;
                    if (mappedNum > 0)
                    {
                        // Push to supernode
                        baseGain = (1-alpha)*residue2[curNode]/(inDeg[curNode] - mappedNum + 1);
                        residue2[n] += baseGain;
                        if (residue2[n] / inDeg[n] >= rmax)
                            candidatePush2.push(n);
                        else
                            candidatePush2_next.push(n);
                    }
                    else
                        baseGain = (1-alpha)*residue2[curNode]/inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        // Push to ordinary neighbor
                        int adj = bwdG[curNode][i];
                        if (!(nodetosuper2[adj] < -1 && nodetosuper2[adj] > mapCnt2))
                        {
                            residue2[adj] += baseGain;
                            if (residue2[adj] / inDeg[adj] >= rmax)
                                candidatePush2.push(adj);
                            else
                                candidatePush2_next.push(adj);
                        }
                    }
                    residue2[curNode] = 0;
                }
                else
                {
                    candidatePush2_next.push(curNode);
                }
            }
            if(candidatePush2_next.empty())
                return -1;
            
            if (rmax < init_rmax)
            {
                // Set mapping
                int superDeg = 0;
                unordered_set<int> superNeigh2;
                while (!candidatePush2_next.empty())
                {
                    int curNode = candidatePush2_next.front();
                    candidatePush2_next.pop();
                    int bwdSz = inDeg[curNode];
                    for (int i = 0; i < bwdSz; i++)
                    {
                        int adj = bwdG[curNode][i];
                        if (l2[adj] != 0 && superNeigh2.find(adj) == superNeigh2.end())
                        {
                            superNeigh2.insert(adj);
                            bwdG[n][superDeg] = adj;
                            superDeg++;
                        }
                    }
                    explored2[curNode] = true;
                    nodetosuper2[curNode] = mapCnt2;
                }
                inDeg[n] = superDeg;

                // Clear counters
                mLeft2 -= intEdges2;
                intEdges2 = 0;

                // Re-initialize
                candidatePush2.push(n);
                residue2[n] = 1;
                l2[n] = 0;
                explored2[n] = false;
                rmax = start_rmax;

                // Decrease counter
                mapCnt2--;
            }
            else
                swap(candidatePush2,candidatePush2_next);

            // candidatePush1_next, candidatePush2_next are empty
            // All contracted vertices are marked as explored

            // Cost model for switching to BBFS (break).
            cost_push = 2 * lambda * (1.0 / (alpha * init_rmax) - 1.0 / (alpha * rmax)) \
                + lambda * (nLeft1 + nLeft2) / (coeff / (alpha * (1 - alpha) * init_rmax)) * (1.0 / (alpha * init_rmax) - 1.0 / (alpha * start_rmax));
            mLeft1 -= intEdges1;
            mLeft2 -= intEdges2;
            cost_bbfs = (nLeft1 + mLeft1) + (nLeft2 + mLeft2);
            if (cost_push > cost_bbfs)
                break;

            rmax /= 10;
        }

        // BBFS
        while(!candidatePush1.empty() && !candidatePush2.empty())
        {
            while(!candidatePush1.empty())
            {
                int curNode = candidatePush1.front();
                candidatePush1.pop();
                int fwdSz = outDeg[curNode], outNode;
                for (int i = 0; i < fwdSz; i++)
                {
                    outNode = fwdG[curNode][i];
                    if(!explored1[outNode])
                    {
                        candidatePush1_next.push(outNode);
                        explored1[outNode] = true;
                        l1[outNode] = 0;
                        if (l2[outNode] != -1)
                            return 0;
                    }
                }  
            }
            while(!candidatePush2.empty())
            {
                int curNode = candidatePush2.front();
                candidatePush2.pop();
                int bwdSz = inDeg[curNode], inNode;
                for (int i = 0; i < bwdSz; i++)
                {
                    inNode = bwdG[curNode][i];
                    if(!explored2[inNode])
                    {
                        candidatePush2_next.push(inNode);
                        explored2[inNode] = true;
                        l2[inNode] = 0;
                        if (l1[inNode] != -1)
                            return 0;
                    }
                }
            }
            if(candidatePush1_next.empty() || candidatePush2_next.empty())
                return -1;

            swap(candidatePush1_next , candidatePush1);
            swap(candidatePush2_next , candidatePush2);
        }

        return -1;
    }

    void reachOracle(int sourceNode, int targetNode, int mode, double init_rmax, int maxRounds=2)
    {

        int res;
        struct timespec start_at, end_at;
        double minTime = 999999, curTime;

        InitPara(sourceNode, targetNode);
        clock_gettime(CLOCK_MONOTONIC, &start_at);
        BBFS(sourceNode, targetNode);
        clock_gettime(CLOCK_MONOTONIC, &end_at);
        curTime = timeDiff(start_at, end_at);
        // cout << "----------" << endl;
        if (curTime < minTime)
        {
            minTime = curTime;
            // cout << "minTime BiBFS, " << minTime << "ms" << endl;
        }

        for (int i = 1; i <= maxRounds; i++)
        {
            InitPara(sourceNode, targetNode);
            clock_gettime(CLOCK_MONOTONIC, &start_at);
            PushContractTraversal(sourceNode, targetNode, init_rmax, i);
            clock_gettime(CLOCK_MONOTONIC, &end_at);
            curTime = timeDiff(start_at, end_at);
            if (curTime < minTime)
            {
                minTime = curTime;
                // cout << "minTime rounds = " << i << ", " << minTime << "ms" << endl;
            }
        }

        InitPara(sourceNode, targetNode);
        clock_gettime(CLOCK_MONOTONIC, &start_at);
        PushContractTraversal(sourceNode, targetNode, init_rmax);
        clock_gettime(CLOCK_MONOTONIC, &end_at);
        curTime = timeDiff(start_at, end_at);
        if (curTime < minTime)
        {
            minTime = curTime;
            // cout << "minTime ours, " << minTime << "ms" << endl;
        }
        // cout << "----------" << endl;

        timeQueries += minTime;
    }
    
    bool reachfinal(int sourceNode, int targetNode, int mode, double init_rmax)
    {
        InitPara(sourceNode, targetNode);

        int res;
        struct timespec start_at, end_at;
        clock_gettime(CLOCK_MONOTONIC, &start_at);
        switch (mode)
        {
            case 0: res = BBFS(sourceNode, targetNode); break;
            case 1: res = Push(sourceNode, targetNode, init_rmax); break;
            case 2: res = PushContract(sourceNode, targetNode, init_rmax); break;
            case 3: res = PushContractTraversal(sourceNode, targetNode, init_rmax); break;
        }
        clock_gettime(CLOCK_MONOTONIC, &end_at);
        double elapsed = timeDiff(start_at, end_at);
        timeQueries += elapsed;
        if (res >= 0)
        {
            timePosQueries += elapsed;
            return true;
        }
        else
        {
            timeNegQueries += elapsed;
            return false;
        }
    }
};

#endif
