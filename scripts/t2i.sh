# L_list=(10 20 30 40 50 60 70 80 90 100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600)
# thread_num=16

L_list=(10 20 30 40 50 60 70 80 90 100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600)
thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/Text-to-Image/base.10M.fbin \
/SSD/Text-to-Image/query.10k.fbin \
/SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/t2i_8t_1stepsync.csv
