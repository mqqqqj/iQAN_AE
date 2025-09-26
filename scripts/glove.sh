# L_list=(100 200 300 400 450 500 550 600)
# L_list=(100 120 150 170 200 220 250 270 300)
# L_list=(100 200 300 400 500 600 700 800 1000 1200 1400 1800 2200)
L_list=(100 150 200 250 300 350 400 450 500 550 600)
# L_list=(100)

# L_list=(700 1400 2100)

thread_num=16

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/Glove200/base.1m.fbin \
/SSD/Glove200/query.fbin \
/SSD/models/nsg/glove200.L2000.R64.C200.nsg \
100 output.ivecs /SSD/Glove200/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/plot/glove_iqan_8t_0_8Lsync.csv
#  | tee -a /home/mqj/proj/ANNSLib/plot/glove_iqan_8t_100stepsync.csv
#/home/mqj/proj/ANNSLib/results/laion_iqan_4t_0831.csv
