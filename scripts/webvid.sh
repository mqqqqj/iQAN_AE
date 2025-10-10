L_list=(100 150 200 250 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600)
# L_list=(1800 2000 2200 2400 2600 2800)

thread_num=16

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/WebVid/webvid.base.2.5M.fbin \
/SSD/WebVid/webvid.query.10k.fbin \
/SSD/models/nsg/webvid.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/WebVid/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/webvid_16t_iqan.csv
