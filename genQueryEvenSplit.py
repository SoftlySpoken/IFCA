# ./genQueryBatch ~/khop_reachability/expr_impl/datasets/toy toy 2 10
datasets = ["enron", "epinions", "digg-friends", "flickr-growth", "friendster", \
"wiki-talk", "wiki-growth", "dynamic-dewiki", "dynamic-frwiki"]
# datasets = ["enron"]
num_query_per_batch = 50000
num_batch = 5
import os
for d in datasets:
    os.system("./genQueryBatch ~/khop_reachability/expr_impl/datasets/" + d + ' ' + d \
    + ' ' + str(num_batch) + ' ' + str(num_batch * num_query_per_batch))