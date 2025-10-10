thread_num=8

L_list=(10 20 30 40 50 60 70 80 90)

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/Glove200/base.1m.fbin \
/SSD/Glove200/query.fbin \
/SSD/models/nsg/glove200.L2000.R64.C200.nsg \
100 output.ivecs /SSD/Glove200/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/glove_8t_iqan.csv

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/LAION/LAION_base_imgemb_10M.fbin \
/SSD/LAION/LAION_test_query_textemb_50k.fbin \
/SSD/models/nsg/laion.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/LAION/gt.test50K.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/laion_8t_iqan.csv

# L_list=(100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600 650 700 750 800 900 1000 1100)

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/mainsearch_8t_iqan.csv

# L_list=(100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600)

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/Text-to-Image/base.10M.fbin \
/SSD/Text-to-Image/query.10k.fbin \
/SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/t2i_8t_iqan.csv

# L_list=(100 125 150 175 200 225 250 275 300 325 350 375 400 425 450 475 500 525 550 575 600 700 800 900 1000 1200 1400 1600)

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/WebVid/webvid.base.2.5M.fbin \
/SSD/WebVid/webvid.query.10k.fbin \
/SSD/models/nsg/webvid.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/WebVid/gt.query.top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/webvid_8t_iqan.csv
