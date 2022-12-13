#include "Base.h"

using namespace std;

unsigned numPosQuery = 0, numNegQuery = 0;

void RunFinal(Base& g , double c, int mode)
{
    struct timespec start_at, end_at;
    int sNode , tNode , hopCnt;
    uint64_t cost = 0.0 ;
    double rmax = c;
    fflush(stdout);

    int qCnt = 0;
    for( auto q : g.QueryList ){
        tie(sNode,tNode,hopCnt) = q;
        if (sNode == tNode)
            continue;

        bool res = g.reachfinal(sNode, tNode, mode, rmax);

        if (hopCnt < 0) {
            numNegQuery++;
            // if (res) {
            //     cout<<"[ERROR] False positive, s = " << sNode << ", t = " << tNode << ", hopCnt = " << hopCnt << endl;
            //     exit(0);
            // }
        } else {
            numPosQuery++;
            // if (!res) {
            //     cout << "False negative, s = " << sNode << ", t = " << tNode << ", hopCnt = " << hopCnt << endl;
            //     g.falseNegatives++;
            // }
        }

        qCnt++;
        if (qCnt % 1000 == 0)
        {
            cout << "query fin " << qCnt << endl;
            double prec = (double)(g.numQueries + qCnt - g.falseNegatives) / (double)(g.numQueries + qCnt);
            printf("Precision = %lf, ", prec);
            printf("Query time = %lf ms\n", g.timeQueries);
        }
    }
    g.numQueries += g.QueryList.size();
}

int main(int argc, char **argv)
{
	if (!(argc >= 6 && argc <= 9))
    {
        printf("Usage: %s path_to_file graph_name num_partitions mode rmax [alpha=0.2] [start_ratio=100] [step=10]\n", argv[0]);
        printf("You entered:");
        for (int i = 0; i < argc; i++)
            printf("%s ", argv[i]);
        printf("\n");
        return -1;
    }

    char graphFile[FILELEN], updateFile[FILELEN], queryFile[FILELEN];
    sprintf(graphFile, "%s/graph.%s", argv[1], argv[2]);
    sprintf(updateFile, "%s/update.%s", argv[1], argv[2]);
    sprintf(queryFile, "%s/query.%s", argv[1], argv[2]);
    int num_partitions = atoi(argv[3]);
    int mode = atoi(argv[4]);
	int querySize = 1000;  // Meaningless except for generating query
    double theta = 1;
    double alpha = 0.2, start_ratio = 100, step = 10;
    // double currRmax = 1e-7;  // Only meaningful for Push (approximate method)
    double currRmax = atof(argv[5]);
    if (argc >= 7)
    {
        alpha = atof(argv[6]);
        if (argc >= 8)
        {
            start_ratio = atof(argv[7]);
            if (argc == 9)
                step = atof(argv[8]);
        }
    }

	Base g(graphFile, updateFile, queryFile, querySize, theta, alpha, num_partitions, start_ratio, step);

    // Update and query in batches
    char currUpdateFile[FILELEN], currQueryFile[FILELEN];
    printf("Command: ");
    for (int i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");
    printf("----------\nMode = %d, rmax = %f, alpha = %f\n", mode, currRmax, alpha);
    double prec;

    struct timespec start_at, end_at;
    clock_gettime(CLOCK_MONOTONIC, &start_at);
    Base curr_g = g;
    clock_gettime(CLOCK_MONOTONIC, &end_at);
    printf("graph copy cost: %lf ms\n", curr_g.timeDiff(start_at, end_at));

    for (int i = 0; i < num_partitions; i++)
    {
        if (i != 0)
        {
            // Update
            printf("%d-th update...\n", i);
            sprintf(currUpdateFile, "%s%d", updateFile, i);
            curr_g.loadUpdateFromFile(currUpdateFile);
        }

        // Query
        printf("%d-th query...\n", i);
        sprintf(currQueryFile, "%s%d", queryFile, i);
        curr_g.loadQueryFromFile(currQueryFile);
        RunFinal(curr_g, currRmax, mode);

        // Obtain prec
        prec = (double)(curr_g.numQueries - curr_g.falseNegatives) / (double)(curr_g.numQueries);
        printf("Precision = %lf (%d / %d)\n", prec, curr_g.numQueries - curr_g.falseNegatives, curr_g.numQueries);
        printf("Update time = %lf s, #Updates = %d, Avg update time = %lf\n", curr_g.timeUpdates, curr_g.numUpdates, \
            (double)(curr_g.timeUpdates) / (double)(curr_g.numUpdates));
        printf("Query time = %lf ms, #Queries = %d, Avg query time = %lf\n", curr_g.timeQueries, curr_g.numQueries, \
            (double)(curr_g.timeQueries) / (double)(curr_g.numQueries));
        printf("Positive query time = %lf ms, #Queries = %d, Avg positive query time = %lf\n", curr_g.timePosQueries, curr_g.numQueries - curr_g.falseNegatives, \
            (double)(curr_g.timePosQueries) / (double)(numPosQuery));
        printf("Negative query time = %lf ms, #Queries = %d, Avg negative query time = %lf\n", curr_g.timeNegQueries, curr_g.falseNegatives, \
            (double)(curr_g.timeNegQueries) / (double)(numNegQuery));
    }

    return 0;
}
