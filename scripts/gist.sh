L_list=(100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/GIST1M/base.fbin \
/SSD/GIST1M/query.fbin \
/SSD/models/nsg/gist1m.L2000.R64.C200.nsg \
100 output.ivecs /SSD/GIST1M/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/gist_8t_iqan.csv
