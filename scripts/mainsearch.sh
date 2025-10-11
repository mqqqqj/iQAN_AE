L_list=(10 20 30 40 50 60 70 80 90)

thread_num=16

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_16t_iqan.csv
