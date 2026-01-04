L_list=(100 100 100 100 100 100 100 120 140 160 180 200 220 240 260 280 300 320 340 360 380 400 420 440 460 480 500 520 540 560 580 600 620 640 660 680 700)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/DEEP10M/base.100M.fbin \
/SSD/DEEP10M/query.fbin \
/SSD/models/nsg/deep100m.L200.R64.C200.nsg \
100 output.ivecs /SSD/DEEP10M/gt.100M.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/iQAN_AE/results/deep100m/deep100m_8t_iqan_nosync.csv

