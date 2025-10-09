L_list=(100 150 200 250 300 350 375 400 425 450 475 500 525 550 575 600)

thread_num=2

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/OpenAI-1M/base.fbin \
/SSD/OpenAI-1M/query.fbin \
/SSD/models/nsg/openai1m.L500.R64.C500.nsg \
100 output.ivecs /SSD/OpenAI-1M/groundtruth.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/openai_8t_iqan.csv
