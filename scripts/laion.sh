# L_list=(100 200 300 400 450 500 550 600)
# L_list=(100 120 150 170 200 220 250 270 300)
# L_list=(350 400 450 500 550 600)

# L_list=(700 800 1000 1200 1400 1800 2200)

L_list=(110)

# L_list=(700 1400 2100)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/LAION/LAION_base_imgemb_10M.fbin \
/SSD/LAION/LAION_test_query_textemb_50k.fbin \
/SSD/models/nsg/laion.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/LAION/gt.test50K.bin $thread_num "$L_str"

# | tee -a /home/mqj/proj/ANNSLib/results/laion_iqan_8t.csv

#/home/mqj/proj/ANNSLib/results/laion_iqan_4t_0831.csv
