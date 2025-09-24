# L_list=(100 200 300 400 450 500 550 600)
# L_list=(100 120 150 170 200 220 250 270 300)
# L_list=(350 400 450 500 550 600)

# L_list=(700 800 1000 1200 1400 1800 2200)

L_list=(100)

# L_list=(700 1400 2100)

thread_num=4

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/Text-to-Image/base.10M.fbin \
/SSD/Text-to-Image/query.10k.fbin \
/SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/observation/t2i_4thread_two_stage_hop.csv

# | tee -a /home/mqj/proj/ANNSLib/results/laion_iqan_8t.csv

#/home/mqj/proj/ANNSLib/results/laion_iqan_4t_0831.csv
