thread_num=4

# L_list=(30 35)

# L_str=$(IFS=,; echo "${L_list[*]}")

# taskset -c 0-17,36-53 PSS_v5_distance_threshold_profiling \
# /SSD/Glove200/base.1m.fbin \
# /SSD/Glove200/query.fbin \
# /SSD/models/nsg/glove200.L2000.R64.C200.nsg \
# 100 output.ivecs /SSD/Glove200/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/glove_32t_iqan.csv

# L_list=(10 15 20)

# L_str=$(IFS=,; echo "${L_list[*]}")

# taskset -c 0-17,36-53 PSS_v5_distance_threshold_profiling \
# /SSD/MainSearch/base.fbin \
# /SSD/MainSearch/query_test_unique.fbin \
# /SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_32t_iqan.csv


L_list=(80)

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-3 PSS_v5_distance_threshold_profiling \
/SSD/Text-to-Image/base.10M.fbin \
/SSD/Text-to-Image/query.10k.fbin \
/SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/t2i_4t_iqan.csv