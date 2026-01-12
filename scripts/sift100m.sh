# L_list=(10 20 30 40 50 60 70 80 90 100 150 200 250 300 350 400 450 500 550 600 650 700)
# L_list=(100 150 200 250 300 350 400 450 500 550 600 650 700 750 800 850 900 950 1000 1050 1100 1150 1200 1300 1400 1500 1600 1700 1800 1900 2000)

export LD_LIBRARY_PATH=/home/mqj/bin/gcc-13.2.0/output/lib64:$LD_LIBRARY_PATH

L_list=(560 560 560)

thread_num=8

L_str=$(IFS=,; echo "${L_list[*]}")
perf record -g -F 99 \
taskset -c 0-$((thread_num - 1)) /home/mqj/proj/iQAN_AE/cmake-build-release/PSS_v5_distance_threshold_profiling \
/SSD/SIFT1B/base.100M.u8bin \
/SSD/SIFT1B/query.public.10K.u8bin \
/SSD/models/nsg/sift100m.L200.R64.C200.nsg \
100 output.ivecs /SSD/SIFT1B/gt.public.10K_100M_top100.bin $thread_num "$L_str" #| tee -a /home/mqj/proj/ANNSLib/experiments/revision/sift100m_8t_iqan_nosync.csv

perf report -n --sort=percent,calls