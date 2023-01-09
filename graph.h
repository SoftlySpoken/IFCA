#ifndef GRAPH_H
#define GRAPH_H



#include "Lib.h"

#define FILELEN 200

using namespace std;

class Graph
{
public:
    int n;
    long long  m;
    vector< vector<int> > fwdG;
    vector< vector<int> > bwdG;
    vector<int> outDeg;
    vector<int> inDeg;

    vector<int> bfsMark;
    vector<int> deadNodes;
    vector<int> inDeadNodes;
    vector< tuple<int,int,int> > QueryList;
    Flip flip;

    string datasetPath;
    string updatePath;
    string queryPath;
    FILE *updatefile;
    FILE *queryfile;
    int querySize;

    int numUpdates, numQueries, falseNegatives;
    double timeUpdates, timeQueries, timePosQueries, timeNegQueries;

    vector<double> layer_ratio;

    Graph(string dPath, string uPath, string qPath, int qSize)
    {
        datasetPath = dPath;
        updatePath = uPath;
        queryPath = qPath;
        querySize = qSize;
        
        updatefile = fopen(updatePath.c_str(), "r");
        queryfile = fopen(queryPath.c_str(), "r");

        numUpdates = 0;
        numQueries = 0;
        falseNegatives = 0;
        timeUpdates = 0;
        timeQueries = 0;
        timePosQueries = 0;
        timeNegQueries = 0;
    }

    void LoadGraph(int num_partitions)
    {
        split_line();
        cout<<"Graph Loading"<<endl;
        struct timespec start_at, end_at;
        double timeCost;
        clock_gettime(CLOCK_MONOTONIC, &start_at);
        n = 0;
        m = 0;
        cout<<"dataset path: "<<datasetPath<<endl;

        int fromNode, toNode, currTimestamp;
        char type;
        char currUpdateFile[200];
        for (int i = 1; i < num_partitions; i++)
        {
            sprintf(currUpdateFile, "%s%d", updatePath.c_str(), i);
            ifstream ufile(currUpdateFile);
            assert(ufile.is_open());
            while (true)
            {
                type = ufile.get();
                if (type != 'I' && type != 'D')
                    break;
                ufile >> fromNode >> toNode >> currTimestamp;
                ufile.get();    // \n
                if (n < fromNode + 1)
                    n = fromNode + 1;
                if (n < toNode + 1)
                    n = toNode + 1;
            }
            ufile.close();
        }

        ifstream infile(datasetPath);
        assert(infile.is_open());
        string tmp;
        size_t fromNodePos, toNodePos;
        while (getline(infile, tmp)) {
            if (tmp[0] == '%') continue;
            fromNodePos = tmp.find(' ');
            if (fromNodePos == string::npos) continue;
            toNodePos = tmp.find(' ', fromNodePos + 1);
            if (toNodePos == string::npos) toNodePos = tmp.length();
            fromNode = stoi(tmp.substr(0, fromNodePos));
            toNode = stoi(tmp.substr(fromNodePos + 1, toNodePos - fromNodePos - 1));
            if (n < fromNode + 1) n = fromNode + 1;
            if (n < toNode + 1) n = toNode + 1;
        }
        infile.close();

        cout << "n = " << n << endl;
        for(int i = 0; i <= n; i++)
        {    // Add slot for supernode
            fwdG.push_back( vector<int>() );
            bwdG.push_back( vector<int>() );
            inDeg.push_back(0);
            outDeg.push_back(0);
        }
        bfsMark.resize(n + 1, 0);
        fwdG[n].resize(n, -1);
        bwdG[n].resize(n, -1);

        infile.open(datasetPath);
        while (getline(infile, tmp)) {
            if (tmp[0] == '%') continue;
            fromNodePos = tmp.find(' ');
            if (fromNodePos == string::npos) continue;
            toNodePos = tmp.find(' ', fromNodePos + 1);
            if (toNodePos == string::npos) toNodePos = tmp.length();
            fromNode = stoi(tmp.substr(0, fromNodePos));
            toNode = stoi(tmp.substr(fromNodePos + 1, toNodePos - fromNodePos - 1));
            fwdG[fromNode].push_back(toNode);
            outDeg[fromNode]++;
            bwdG[toNode].push_back(fromNode);
            inDeg[toNode]++;
            m++;
        }
        cout<<"m = "<<m<<endl;
        infile.close();
        cout<<"avg degree: "<<(double)m*2/n<<endl;
        int graphType = 0;
        int errorNode = 0;
        int maxOutDeg = 0;
        int maxInDeg = 0;
        for( int i = 0 ; i < n ; i++ ){
            if( outDeg[i] == 0 ){
                deadNodes.push_back(i);
            }
            if( inDeg[i] == 0 ){
                inDeadNodes.push_back(i);
            }
            if( outDeg[i] == 0 && inDeg[i] == 0 ){
                errorNode++;
            }
            if( outDeg[i] != inDeg[i] ){
                graphType++;
            }
            if( outDeg[i] > maxOutDeg){
                maxOutDeg = outDeg[i];
            }
            if( inDeg[i] > maxInDeg){
                maxInDeg = inDeg[i];
            }
        }
        if( graphType == 0 ){
            cout<<"undirected graph"<<endl;
        }else{
            cout<<"directed graph: "<< graphType <<endl;
        }
        cout<<"max outdegree: "<<maxOutDeg<<" "<<"max indegree: "<<maxInDeg<<endl;
        cout<<"error nodes num: "<<errorNode<<endl;
        cout<<"out dead nodes num: "<<deadNodes.size()<<endl;
        cout<<"in dead nodes num: "<<inDeadNodes.size()<<endl;
        clock_gettime(CLOCK_MONOTONIC, &end_at);
        timeCost = timeDiff(start_at, end_at);
        cout << "graph load cost: "<<timeCost<<"ms"<<endl;
        split_line();
    }
    
    int loadQueryFromFile(char *currQueryFile){
        FILE *queryPtr = fopen(currQueryFile, "r");
        QueryList = vector< tuple<int,int,int> >();
        
        int u, v, timestamp = -1, currTimestamp, hopCnt;
        double positiveNum = 0.0 ;
        while (true)
        {
            if (feof(queryPtr))
            {
                // printf("End of queryfile\n");
                break;
            }
            char t;
            t = fgetc(queryPtr);
            if (t != 'Q')
            {
                printf("Error: line does not start with Q in queryfile\n");
                break;
            }
            if (fscanf(queryPtr, "%d%d%d%d%d", &u, &v, &currTimestamp, &currTimestamp, &hopCnt) != 5)
            {
                printf("Error: line format incorrect in queryfile\n");
                break;
            }
            fgetc(queryPtr);

            QueryList.push_back(tie(u, v, hopCnt));
            if (hopCnt >= 0)
                positiveNum++;
        }

        fclose(queryPtr);
        return timestamp;
    }

    int loadUpdateFromFile(char *currUpdateFile)
    {
        struct timespec start_at, end_at;
        double update_time = 0;
        auto preNumUpdates = numUpdates;
        FILE *updatePtr = fopen(currUpdateFile, "r");
        char t;
        int u, v, currTimestamp;

        while (true)
        {
            if (feof(updatePtr))
                break;
            t = fgetc(updatePtr);
            if (t != 'I' && t != 'D') // Error: such lines should not appear
            {
                // printf("Error: line does not start with I or D in updatefile\n");
                // fseek(updatefile, prevLine, SEEK_SET);
                break;
            }
            if (fscanf(updatePtr, "%d%d%d", &u, &v, &currTimestamp) != 3)  // Error: such lines should not appear
            {
                printf("Error: line format incorrect in updatefile\n");
                // fseek(updatefile, prevLine, SEEK_SET);
                break;
            }
            fgetc(updatePtr);  // \n
            // prevLine = ftell(updatefile);

            clock_gettime(CLOCK_MONOTONIC, &start_at);
            if (t == 'I')
            {
                if (find(fwdG[u].begin(), fwdG[u].end(), v) == fwdG[u].end())
                {
                    fwdG[u].push_back(v);
                    outDeg[u]++;
                }
                if (find(bwdG[v].begin(), bwdG[v].end(), u) == bwdG[v].end())
                {
                    bwdG[v].push_back(u);
                    inDeg[v]++;
                }
            }
            else if (t == 'D')
            {
                if (find(fwdG[u].begin(), fwdG[u].end(), v) != fwdG[u].end())
                {
                    fwdG[u].erase(find(fwdG[u].begin(), fwdG[u].end(), v));
                    outDeg[u]--;
                }
                if (find(bwdG[v].begin(), bwdG[v].end(), u) != bwdG[v].end())
                {
                    bwdG[v].erase(find(bwdG[v].begin(), bwdG[v].end(), u));
                    inDeg[v]--;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &end_at);
            numUpdates++;
            timeUpdates += timeDiff(start_at, end_at) / 1000.0;
            update_time += timeDiff(start_at, end_at);
            // No need to maintain other data structures, because not used in main procedure
        }

        fclose(updatePtr);
        printf("Avg Update time = %lf ms\n", update_time / (double)(numUpdates - preNumUpdates));
        printf("Update time = %lf ms, cur numUpdates = %d\n", update_time, numUpdates - preNumUpdates);
        return currTimestamp;
    }

    int BFS(int sourceNode, int targetNode)
    {
        int layer = 0;
        queue<int> fwdQ;
        queue<int> fwdQ_next;
        fwdQ.push(sourceNode);
        for (int i = 0; i < n; i++)
            bfsMark[i] = 0;
        bfsMark[sourceNode] = 1;

        while(1)
        {
            if (fwdQ.empty())
            {
                if (!fwdQ_next.empty())
                {
                    swap(fwdQ_next, fwdQ);
                    layer++;
                }
                else
                    break;
            }

            int curNode = fwdQ.front();
            fwdQ.pop();
            for( int adj : fwdG[curNode] )
            {
                if (bfsMark[adj] == 0)
                {
                    fwdQ_next.push(adj);
                    if( adj == targetNode )
                        return layer + 1;
                    bfsMark[adj] = 1;
                }
            }
        }

        return -1;
    }

    void getBfsVerts(int sourceNode, unordered_set<int> &ret) {
        queue<int> q;
        q.push(sourceNode);
        ret.clear();
        ret.emplace(sourceNode);

        int cur;
        while (!q.empty()) {
            cur = q.front();
            q.pop();
            for (auto adj : fwdG[cur]) {
                if (ret.find(adj) == ret.end()) {
                    q.push(adj);
                    ret.emplace(adj);
                }
            }
        }
    }

    void GenerateQueryToFile(char *currQueryFile, int num_part_query, int timestamp)
    {
        int positiveNum = 0;
        int sNode, tNode, hopCnt;
        bool s = false;
        bool t = false;
        FILE *queryPtr = fopen(currQueryFile, "w");

        for (int i = 0; i < num_part_query; i++)
        {
            // Sample node pair
            sNode = flip.Gen_Max() % n;
            tNode = flip.Gen_Max() % n;
            if (sNode == tNode || outDeg[sNode] == 0 || inDeg[tNode] == 0)
            {
                i--;
                continue;
            }

            // Compute hop count
            bfsMark = vector<int>(n,0);
            hopCnt = BFS(sNode, tNode);
            if (hopCnt >= 0)
                positiveNum++;

            fprintf(queryPtr, "Q %d %d %d %d %d\n", sNode, tNode, timestamp, timestamp, hopCnt);
        }

        fclose(queryPtr);

        printf("#positive queries = %d, #negative queries = %d\n", positiveNum, num_part_query - positiveNum);
    }

    void GenerateQueryToFileEvenSplit(char *currQueryFile, int num_part_query, int timestamp)
    {
        int positiveNum = 0;
        int sNode, tNode, hopCnt;
        bool s = false;
        bool t = false;
        FILE *queryPtr = fopen(currQueryFile, "w");

        // Collate pos & neg separately
        vector<tuple<int, int, int>> posQuery, negQuery;

        // for (int i = 0; i < num_part_query; i++)
        while (posQuery.size() < num_part_query) {
            // Sample node pair
            sNode = flip.Gen_Max() % n;
            if (outDeg[sNode] == 0) continue;
            tNode = flip.Gen_Max() % n;
            if (sNode == tNode || inDeg[tNode] == 0) continue;

            // Compute hop count
            bfsMark = vector<int>(n,0);
            hopCnt = BFS(sNode, tNode);

            if (hopCnt >= 0) {
                positiveNum++;
                posQuery.emplace_back(sNode, tNode, hopCnt);
            } else if (negQuery.size() < num_part_query)
                negQuery.emplace_back(sNode, tNode, hopCnt);
        }

        // If neg not enough, generate more neg
        // Find reachable vertices first, then draw randomly until not in reachable set
        int batchSz, curSz;
        int maxBatchSz = num_part_query / 250;
        if (maxBatchSz < 1) maxBatchSz = 1;
        unordered_set<int> reachable;
        // cout << negQuery.size() << ' ' << num_part_query << endl;
        while (negQuery.size() < num_part_query) {
            sNode = flip.Gen_Max() % n;
            if (outDeg[sNode] == 0) {
                // cout << "Case 1: " << sNode << endl;
                continue;
            }
            reachable.clear();
            getBfsVerts(sNode, reachable);
            if (reachable.size() > n / 2) {
                // cout << "Case 2: " << sNode << endl;
                continue;
            }
            curSz = 0;
            batchSz = flip.Gen_Max() % maxBatchSz;
            if (batchSz < 1) batchSz = 1;
            while (curSz < batchSz && negQuery.size() < num_part_query) {
                tNode = flip.Gen_Max() % n;
                if (!reachable.empty() && reachable.find(tNode) != reachable.end()) continue;
                curSz++;
                negQuery.emplace_back(sNode, tNode, -1);
                // cout << sNode << ", " << tNode << endl;
            }
        }

        // Shuffle neg queries
        std::random_device rd;
        std::default_random_engine rng(rd());
        shuffle(negQuery.begin(), negQuery.end(), rng);
        
        // Write to file
        for (auto pos : posQuery)
            fprintf(queryPtr, "Q %d %d %d %d %d\n", get<0>(pos), get<1>(pos), 
                timestamp, timestamp, get<2>(pos));
        for (auto neg : negQuery)
            fprintf(queryPtr, "Q %d %d %d %d %d\n", get<0>(neg), get<1>(neg), 
                timestamp, timestamp, get<2>(neg));

        fclose(queryPtr);

        printf("#positive queries = %d, #negative queries = %d\n", posQuery.size(), negQuery.size());
    }

    double timeDiff(struct timespec start_at, struct timespec end_at)
    {
        // Return time difference in milliseconds
        double temp_sec, temp_nsec;
        if (end_at.tv_nsec - start_at.tv_nsec < 0)
        {
            temp_sec = end_at.tv_sec - start_at.tv_sec - 1;
            temp_nsec = 1000000000 + end_at.tv_nsec - start_at.tv_nsec;
        }
        else
        {
            temp_sec = end_at.tv_sec - start_at.tv_sec;
            temp_nsec = end_at.tv_nsec - start_at.tv_nsec;
        }
        return (double)temp_sec * 1000.0 + (double)temp_nsec / 1000000.0;
    }

};

#endif
