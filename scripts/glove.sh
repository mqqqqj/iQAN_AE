# L_list=(100 150 200 250 300 350 375 400 425 450 475 500 525 550 575 600 650 700 750 800 900 1000 1100 1200)
# L_list=(210 220 230 240 250 260 270 280 290)
# L_list=(100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500)

L_list=(130 135 140 145)

#100 150 200 250 300 350 375 400

thread_num=32

#0-$((thread_num - 1))

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-17,36-49 PSS_v5_distance_threshold_profiling \
/SSD/Glove200/base.1m.fbin \
/SSD/Glove200/query.fbin \
/SSD/models/nsg/glove200.L2000.R64.C200.nsg \
100 output.ivecs /SSD/Glove200/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/glove_32t_iqan.csv
