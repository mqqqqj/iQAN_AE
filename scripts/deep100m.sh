L_list=(100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 850 900 1000)
# L_list=(100 200 300 400 500 600 700 800 900 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/DEEP10M/base.100M.fbin \
/SSD/DEEP10M/query.fbin \
/SSD/models/nsg/deep100m.L200.R64.C200.nsg \
100 output.ivecs /SSD/DEEP10M/gt.100M.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/deep100m_8t_1stepsync.csv
