# L_list=(100 150 200 250 300 350 400 450 500 600)
# L_list=(100 120 150 170 200 220 250 270 300)
# L_list=(350 400 450 500 550 600)


L_list=(400)

# L_list=(700 1400 2100)

thread_num=4

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/DEEP10M/base.fbin \
/SSD/DEEP10M/query.fbin \
/SSD/models/nsg/deep10m.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/DEEP10M/gt.query.top100.bin $thread_num "$L_str"

#/home/mqj/proj/ANNSLib/results/laion_iqan_4t_0831.csv
