#include "Base.h"

int main(int argc, char **argv)
{
	if (argc != 5)
	{
		printf("Usage: %s dataset_path dataset_name num_partitions num_total_query\n", argv[0]);
		return -1;
	}

	bool evenSplit = false, dag = false;

	char graphFile[FILELEN], updateFile[FILELEN], queryFile[FILELEN];
	if (dag) sprintf(graphFile, "%s/dag.%s", argv[1], argv[2]);
	else sprintf(graphFile, "%s/graph.%s", argv[1], argv[2]);
	// else sprintf(graphFile, "%s/out.%s", argv[1], argv[2]);
	sprintf(updateFile, "%s/update.%s", argv[1], argv[2]);
	if (evenSplit) sprintf(queryFile, "%s/evenSplit/query.%s", argv[1], argv[2]);
	else if (dag) sprintf(queryFile, "%s/dagQuery/query.%s", argv[1], argv[2]);
	else sprintf(queryFile, "%s/query.%s", argv[1], argv[2]);
	int num_partitions = atoi(argv[3]), num_total_query = atoi(argv[4]);
	int num_part_query;
	if (evenSplit) num_part_query = num_total_query / num_partitions * 0.9;
	else num_part_query = num_total_query / num_partitions;

	Graph g(graphFile, updateFile, queryFile, num_part_query);
	g.LoadGraph(num_partitions);

	char currUpdateFile[FILELEN], currQueryFile[FILELEN];
	int timestamp;
	for (int i = 0; i < num_partitions; i++)
	{
		if (i != 0)
		{
			// Update
			printf("%d-th update...\n", i);
			sprintf(currUpdateFile, "%s%d", updateFile, i);
			timestamp = g.loadUpdateFromFile(currUpdateFile);
		}

		// Generate query
		printf("%d-th gen query...\n", i);
		sprintf(currQueryFile, "%s%d", queryFile, i);
		if (evenSplit) g.GenerateQueryToFileEvenSplit(currQueryFile, num_part_query, timestamp);
		else g.GenerateQueryToFile(currQueryFile, num_part_query, timestamp);
	}
}
