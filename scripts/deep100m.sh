L_list=(5 5 5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600 625 650)

thread_num=16

L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/DEEP10M/base.100M.fbin \
/SSD/DEEP10M/query.fbin \
/SSD/models/nsg/deep100m.L200.R64.C200.nsg \
100 output.ivecs /SSD/DEEP10M/gt.100M.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/overall_performance/deep100m_16t_iqan_tuned_new.csv
