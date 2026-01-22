L_list=(100 150 200 250 300 350 400 450 500 550 600 700 800 900 1000)

thread_num=8

#deep100m
L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) no_sync \
/SSD/DEEP10M/base.100M.fbin \
/SSD/DEEP10M/query.fbin \
/SSD/models/nsg/deep100m.L200.R64.C200.nsg \
100 output.ivecs /SSD/DEEP10M/gt.100M.top100.bin $thread_num "$L_str" | tee -a ./deep100m_8t_nosync.csv

#glove200
L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) no_sync \
/SSD/Glove200/base.1m.fbin \
/SSD/Glove200/query.fbin \
/SSD/models/nsg/glove200.L2000.R64.C200.nsg \
100 output.ivecs /SSD/Glove200/gt.query.top100.bin $thread_num "$L_str" | tee -a ./glove200_8t_nosync.csv

#webvid2.5m
L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) no_sync \
/SSD/WebVid/webvid.base.2.5M.fbin \
/SSD/WebVid/webvid.query.10k.fbin \
/SSD/models/nsg/webvid.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/WebVid/gt.query.top100.bin $thread_num "$L_str" | tee -a ./webvid_8t_nosync.csv

#laion10m
taskset -c 0-$((thread_num - 1)) no_sync \
/SSD/LAION/LAION_base_imgemb_10M.fbin \
/SSD/LAION/LAION_test_query_textemb_50k.fbin \
/SSD/models/nsg/laion.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/LAION/gt.test50K.bin $thread_num "$L_str" | tee -a ./laion10m_8t_nosync.csv
