L_list=(100 150 200 250 300 350 400 450 500 550 600)

# L_list=(100 200 300 400 500 600 700 800 1000 1200 1400 1800 2200)

# L_list=(110)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/WebVid/webvid.base.2.5M.fbin \
/SSD/WebVid/webvid.query.10k.fbin \
/SSD/models/nsg/webvid.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/WebVid/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/plot/webvid_iqan_8t.csv

# | tee -a /home/mqj/proj/ANNSLib/results/laion_iqan_8t.csv

#/home/mqj/proj/ANNSLib/results/laion_iqan_4t_0831.csv
