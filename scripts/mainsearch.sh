# L_list=(100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600)
L_list=(650 700 750 800 900 1000 1100 1200)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_8t_iqan.csv
