L_list=(2 3 4 5 10 20 40 60 80 100 150 200 250 300 350 400 450)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
500 output.ivecs /SSD/MainSearch/gt.test_unique_top500.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_8t_iqan_k500.csv

# taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
# /SSD/MainSearch/base.fbin \
# /SSD/MainSearch/query_test_unique.fbin \
# /SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_16t_iqan.csv
