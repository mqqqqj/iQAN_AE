# L_list=(10 20 30 40 50 60 70 80 90 100)
L_list=(100 120 140 160 180 200)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/SIFT1B/slice.10M.u8bin \
/SSD/SIFT1B/query.public.10K.u8bin \
/SSD/models/nsg/sift10m.L200.R32.C200.nsg \
100 output.ivecs /SSD/SIFT1B/gt.public.10K_10M_top100.bin $thread_num "$L_str" #| tee -a /home/mqj/proj/ANNSLib/experiments/glove_8t_iqan.csv


# PSS_v5_find_L_index_adaptive_merge_L_master /SSD/SIFT1B/slice.10M.u8bin \
# /SSD/SIFT1B/query.public.10K.u8bin \
# /SSD/models/nsg/sift10m.L200.R32.C200.nsg \
# 100 output.ivecs /SSD/SIFT1B/gt.public.10K_10M_top100.bin $thread_num \
# 800 


