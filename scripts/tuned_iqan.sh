L_list=(100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 900 1000)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")
taskset -c 0-$((thread_num - 1)) PSS_v5_distance_threshold_profiling \
/SSD/SIFT1B/base.100M.u8bin \
/SSD/SIFT1B/query.public.10K.u8bin \
/SSD/models/nsg/sift100m.L200.R64.C200.nsg \
100 output.ivecs /SSD/SIFT1B/gt.public.10K_100M_top100.bin $thread_num "$L_str" | tee -a ./sift100m_8t_tuned_iqan.csv
