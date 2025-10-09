L_list=(100 150 200 250 300 350 400 450 500)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

# taskset -c 0-$thread_num online_search \
# /SSD/LAION/LAION_base_imgemb_10M.fbin \
# /SSD/LAION/LAION_test_query_textemb_50k.fbin \
# /SSD/models/nsg/laion.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/LAION/gt.test50K.bin $thread_num "$L_str" 500

taskset -c 0-$thread_num online_search \
/SSD/MainSearch/base.fbin \
/SSD/MainSearch/query_test_unique.fbin \
/SSD/models/nsg/mainsearch.L2000.R64.C2000.nsg \
100 output.ivecs /SSD/MainSearch/gt.test_unique.bin $thread_num "$L_str" 1000

# taskset -c 0-$thread_num online_search \
# /SSD/Text-to-Image/base.10M.fbin \
# /SSD/Text-to-Image/query.10k.fbin \
# /SSD/models/nsg/t2i10m.L2000.R64.C2000.nsg \
# 100 output.ivecs /SSD/Text-to-Image/gt.10K_10M.bin $thread_num "$L_str" 1000