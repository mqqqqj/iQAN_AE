# L_list=(100 150 200 250 300 350 400 450 500 550 600)

# L_list=(100)

#10step
# L_list=(100 200 300 400 500 600 675 700 800 1000 1200 1400 1620 1800 2200)

L_list=(100 200 300 400 500 600 700 800 1000 1200 1400 1800 2200)


# L_list=(120 1600)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/Text-to-Image/base.10M.fbin \
/SSD/Text-to-Image/query.10k.fbin \
/SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/plot/t2i_iqan_8t_20stepsync.csv

#/home/mqj/proj/ANNSLib/results/laion_iqan_4t_0831.csv
