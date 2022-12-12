This is the repository accompanying the paper "IFCA: Index-Free Commnunity-Aware Reachability Processing Over Large Dynamic Graphs."

Note: `SFMT-src-1.5.1` is used to choose source and destination vertices uniformly at random during query generation, which is open-source and originally downloaded [here](http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/SFMT/).

### Compile and Run

To compile the query generator:

```bash
$ g++ -O2 -std=c++11 genQueryBatch.cpp SFMT-src-1.5.1/SFMT.c -o genQueryBatch
```

To run the query generator:

```bash
$ ./genQueryBatch dataset_path dataset_name num_partitions num_total_query
```

For example, if you have a dataset in the directory `/home/user/`, the name of the dataset is `some_dataset`, and you would like to partition its time span into 20 intervals and generate 1,000,000 queries in total, then run:

```bash
$ ./genQueryBatch /home/user/ some_dataset 20 1000000
```

To compile the test driver:

```bash
$ g++ -O2 -std=c++11 testBatch.cpp SFMT-src-1.5.1/SFMT.c -o testBatch
```

To run the test driver:

```bash
$ ./testBatch dataset_path dataset_name num_partitions mode
```

`mode` indicates the tested methods (as reported in Section 6.2).

- 0: BiBFS.
- 1: Base.
- 2: Contract.
- 3: IFCA.

### Datasets

One of the real datasets, Enron, is included in this repository (enron.tar.bz2) as an example.

If you would like to run `testBatch` on a custom dataset, please convert it to the following format:

- Initial snapshot: should be named as `graph.[dataset_name]`. For example, if the name of the dataset is `my_dataset`, then the initial snapshot file should be named as `graph.my_dataset`. Each line represents a directed edge in the initial snapshot. For example,

    ```
    1 2
    ```

    represents a directed edge from the vertex indexed as 1 to the vertex indexed as 2. (The vertex indexes should start from 0 and increment as consecutive integers.)

- Update files: the i-th update file should be named as `update.[dataset_name]i`. There can be multiple update files, whose indexes start from 1 and increment as consecutive integers. The update files with larger indexes come later in time. Each line represents an edge insertion or deletion event. For example,

    ```
    I 1 3 1
    ```

    represents the insertion of a directed edge from the vertex indexed as 1 to the vertex indexed as 3 at the timestamp 1; and

    ```
    D 1 3 2
    ```

    represents the deletion of the directed edge from the vertex indexed as 1 to the vertex indexed as 3 at the timestamp 2.

These files should be placed under the same directory, the path of which will be the `dataset_path` for both `genQueryBatch` and `testBatch`; `num_partitions` is equal to (i + 1).

You may use `genQueryBatch` to generate queries, or generate your own queries. The i-th query file corresponding to the i-th snapshot should be named as `query.[dataset_name]i`. Note that the indexes of query files start from 0; the 0-th query file corresponds to the initial snapshot, and the i-th (i > 0) corresponds to the snapshot obtained after applying all the updates in the 1-th, 2-th, ..., and i-th update files to the initial snapshot. Each line represents a query, in the form of:

```
Q source destination timestamp timestamp ground_truth
```

`ground_truth >= 0` indicates that it is a positive query; `ground_truth == -1` indicates that it is a negative query. Note that the `timestamp` is treated as a dummy in the current implementation of `testBatch`; but you could circumvent that if you implement your own driver program for query processing.

