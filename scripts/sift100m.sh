# L_list=(10 20 30 40 50 60 70 80 90 100 150 200 250 300 350 400 450 500 550 600 650 700)
L_list=(100 150 200 250 300 350 400 450 500 550 600 650 700 750 800)
# L_list=(10 20 30 40 50 60 70 80 90)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")

taskset -c 0-$((thread_num - 1)) /home/mqj/proj/iQAN_AE/cmake-build-release/PSS_v5_distance_threshold_profiling \
/SSD/SIFT1B/base.100M.u8bin \
/SSD/SIFT1B/query.public.10K.u8bin \
/SSD/models/hnsw/sift100m-m32-ef500.bin \
100 output.ivecs /SSD/SIFT1B/gt.public.10K_100M_top100.bin $thread_num "$L_str" | tee -a /home/mqj/proj/ANNSLib/experiments/revision/hnsw/sift100m_8t_iqan.csv
