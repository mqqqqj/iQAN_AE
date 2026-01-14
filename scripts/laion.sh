L_list=(100 150 200 250 300 350 400 450 500 550 600 700 800 900 1000 1100 1200 1300 1400 1500)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

# taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
# /SSD/LAION/LAION_base_imgemb_10M.fbin \
# /SSD/LAION/LAION_test_query_textemb_50k.fbin \
# /SSD/models/nsg/laion.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/LAION/gt.test50K.bin $thread_num "$L_str" #| tee -a /home/mqj/proj/ANNSLib/experiments/revision/laion_8t_iqan_nosync.csv

# hnsw
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/LAION/LAION_base_imgemb_10M.fbin \
/SSD/LAION/LAION_test_query_textemb_50k.fbin \
/SSD/models/hnsw_mainse/hnsw_laion_m32_ef2000_ip \
100 output.ivecs /SSD/LAION/gt.test50K.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/hnsw/laion_8t_iqan.csv
