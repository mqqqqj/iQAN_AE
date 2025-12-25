thread_num=8


# L_list=(500 600 700 800)
# L_list=(1500 2000 2400 2800 3200 3600 4000 4400 4800 5200 5600 6000)
L_list=(1600 1700 1800 1850 1900 1950)


L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
500 output.ivecs /SSD/MainSearch/gt.test_unique_top500.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_8t_iqan_1stepsync.csv


# L_list=(1610 1620 1630 1640 1650)
# L_list=(105 110 115 120)
# L_str=$(IFS=,; echo "${L_list[*]}")


# taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
# /SSD/Text-to-Image/base.10M.fbin \
# /SSD/Text-to-Image/query.10k.fbin \
# /SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" #| tee -a /home/mqj/proj/ANNSLib/experiments/t2i_8t_iqan_1stepsync.csv

