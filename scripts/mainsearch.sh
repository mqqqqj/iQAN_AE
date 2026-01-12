L_list=(100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 900 1000 1100)
thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/MainSearch/gt.test_unique_top500.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/mainsearch_8t_iqan_nosync.csv
