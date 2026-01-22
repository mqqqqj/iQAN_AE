# L_list=(100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 900 1000 1100)
# L_list=(10 20 30 40 50 60 70 80 90)
L_list=(110 120 130 140 150 160 170 180 190 200)


thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

# taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
# /SSD/MainSearch/base.fbin \
# /SSD/MainSearch/query_test_unique.fbin \
# /SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/MainSearch/gt.test_unique_top500.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/mainsearch_8t_iqan_nosync.csv

#hnsw
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/hnsw/mainse-m32-ef2000 \
100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/hnsw/mainsearch_8t_iqan.csv
