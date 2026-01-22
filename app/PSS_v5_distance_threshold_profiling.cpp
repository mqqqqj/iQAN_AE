//
// Created by Zhen Peng on 02/14/2021.
//

#include <iostream>
#include <cstdio>
#include <vector>
#include <chrono>
#include <clocale>
#include <omp.h>
#include <sys/resource.h>
#include <sstream>
// #include "../include/papi_panns.h"
// #include "../core/Searching.202002101535.reorganization.h"
// #include "../core/Searching.201912161559.set_for_queue.h"
// #include "../core/Searching.201912091448.map_for_queries_ids.h"
// #include "../core/Searching.h"
// #include "../include/utils.h"
// #include "../include/efanna2e/index_nsg.h"
// #include "../core/Searching.202002141745.critical_omp_top_m.h"
// #include "../core/Searching.202002181409.local_queue_and_merge.h"
// #include "../core/Searching.202002201424.parallel_merge_local_queues.h"
// #include "../core/Searching.202003021000.profile_para_top_m_search.h"
// #include "../core/Searching.202004131634.better_merge.h"
// #include "../core/Searching.202005271122.choosing_m.h"
// #include "../core/Searching.202006191549.nested_parallel.h"
// #include "../core/Searching.202008061153.less_sync.h"
// #include "../core/Searching.202008062049.computation_quota.h"
// #include "../core/Searching.202008090117.interval_merge.h"
// #include "../core/Searching.202008101718.interval_merge_v2.h"
// #include "../core/Searching.202008141252.interval_merge_v4.h"
// #include "../core/Searching.202008152055.interval_merge_v5.h"
// #include "../core/Searching.202008211350.simple_top_m.h"
#include "../core/Searching.202102022027.PSS_v5.dist_thresh.profiling.h"

void usage(int argc, char *argv[])
{
    if (argc != 9)
    {
        fprintf(stderr,
                "Usage: %s <data_file> <query_file> <nsg_path> "
                "<K> <result_file> <true_NN_file> <num_threads> "
                "<search_L_list> \n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
}

unsigned get_L_low(unsigned L, unsigned num_thread)
{
    if (L < 100)
        return L;
    else if (L / num_thread < 100)
        return 100;
    else
        return L / num_thread;
}

unsigned get_X_low(unsigned L, unsigned num_thread)
{
    // float f_L = L;
    // float f_T = num_thread;
    if (L / num_thread < 1)
        return 1;
    else
        return L / num_thread;
}

int main(int argc, char **argv)
{
    usage(argc, argv);
    setbuf(stdout, nullptr);   // Remove stdout buffer.
    setlocale(LC_NUMERIC, ""); // For comma number format
    struct rlimit limit;
    limit.rlim_cur = 0;
    limit.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &limit);
    PANNS::Searching engine;

    engine.load_data_load(argv[1]);
    engine.load_queries_load(argv[2]);
    engine.load_nsg_graph(argv[3]);
    // engine.load_hnsw_graph(argv[3]);

    //    unsigned L_min = strtoull(argv[4], nullptr, 0);
    const unsigned K = strtoull(argv[4], nullptr, 0);
    //    unsigned M_max = strtoull(argv[7], nullptr, 0);
    //    if (L < K) {
    //        fprintf(stderr, "Error: search_L %u is smaller than search_K %u\n.", L, K);
    //        exit(EXIT_FAILURE);
    //    }
    //    if (K < M_max) {
    ////        fprintf(stderr, "Error: search_K %u is smaller than value_M %u.\n", K, M_max);
    ////        exit(EXIT_FAILURE);
    //        fprintf(stderr, "Warning: search_K %u is smaller than value_M %u.\n", K, M_max);
    //    }
    const char *path_results = argv[5];
    std::vector<std::vector<PANNS::idi>> true_nn_list;
    engine.load_groundtruth(
        argv[6],
        true_nn_list);

    unsigned data_dimension = engine.dimension_;
    unsigned points_num = engine.num_v_;

    unsigned query_num = engine.num_queries_;
    if (true_nn_list.size() > query_num)
    {
        true_nn_list.resize(query_num);
    }
    int num_threads = strtoull(argv[7], nullptr, 0);
    engine.num_threads_ = num_threads;
    omp_set_num_threads(num_threads);
    //    omp_set_nested(1);
    //    omp_set_max_active_levels(2);
    std::vector<int> L_list;
    std::string L_str = argv[8];
    std::stringstream ss(L_str);
    std::string L_val;
    while (std::getline(ss, L_val, ','))
    {
        L_list.push_back(std::stoi(L_val));
    }

    printf("L,X,Throughput,latency,recall,p95recall,p99recall,p95latency,p99latency,total_dist_comps,max_dist_comps,hops,avg_merge,t_expand(s.),t_merge(s.),t_seq(s.),t_p_expand(%%),t_p_merge(%%),t_p_seq(%%)\n");
    for (int L : L_list)
    {
        L = L * num_threads;
        const unsigned L_master_low = get_L_low(L, num_threads);
        const unsigned L_master_up = get_L_low(L, num_threads) + 8;
        const unsigned X_low = get_X_low(L, num_threads);
        const unsigned X_up = get_X_low(L, num_threads) + 8;
        const unsigned L_master_step = 2;
        const unsigned L_local_low = 0;
        const unsigned L_local_up = 0;
        const unsigned L_local_step = 0;
        const unsigned X_step = 2;
        const unsigned I_thresh_low = 0;
        const unsigned I_thresh_up = 0;
        const unsigned I_thresh_step = 0;

        for (unsigned L_master = L_master_low; L_master <= L_master_up; L_master += L_master_step)
        {
            unsigned L_local = L_master;
            unsigned Index_thresh = L_local - 1;
            //        unsigned L_local = 100;
            //        unsigned Index_thresh = 99;
            //        for (unsigned L_local = L_local_low; L_local <= L_local_up; L_local += L_local_step) {

            for (unsigned subsearch_iterations = X_low; subsearch_iterations <= X_up; subsearch_iterations += X_step)
            {

                //                for (unsigned Index_thresh = I_thresh_low; Index_thresh <= I_thresh_up; Index_thresh += I_thresh_step) {
                engine.index_thresh_ = Index_thresh;
                unsigned warmup_max = 1;
                for (unsigned warmup = 0; warmup < warmup_max; ++warmup)
                {
                    std::vector<std::vector<PANNS::idi>> set_K_list(query_num);
                    for (unsigned i = 0; i < query_num; i++)
                        set_K_list[i].resize(K);

                    std::vector<PANNS::idi> init_ids(L_master);
                    boost::dynamic_bitset<> is_visited(points_num);
                    std::vector<PANNS::Candidate> set_L((num_threads - 1) * L_local + L_master);
                    std::vector<PANNS::idi> local_queues_sizes(num_threads, 0);
                    std::vector<PANNS::idi> local_queues_starts(num_threads);
                    for (int q_i = 0; q_i < num_threads; ++q_i)
                    {
                        local_queues_starts[q_i] = q_i * L_local;
                    }
                    //                std::vector<PANNS::idi> top_m_candidates(num_threads);
                    std::vector<float> latency_list(query_num); // 单位：毫秒
                    auto s = std::chrono::high_resolution_clock::now();
                    //                engine.prepare_init_ids(init_ids, L_local);
                    engine.prepare_init_ids(init_ids, L_master);
                    for (unsigned q_i = 0; q_i < query_num; ++q_i)
                    {
                        auto start_time = std::chrono::high_resolution_clock::now();
                        engine.para_search_PSS_v5_dist_thresh_profiling(
                            q_i,
                            K,
                            L_master,
                            set_L,
                            init_ids,
                            set_K_list[q_i],
                            L_local,
                            local_queues_starts,
                            local_queues_sizes,
                            is_visited,
                            subsearch_iterations);
                        auto end_time = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                        latency_list[q_i] = duration.count() / 1000.0f; // 转换为毫秒
                    }
                    engine.ub_ratio /= query_num;
                    engine.ub_ratio = 0;
                    auto e = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> diff = e - s;
                    float accumulate_latency = std::accumulate(latency_list.begin(), latency_list.end(), 0.0f);
                    float avg_latency = accumulate_latency / latency_list.size();
                    std::sort(latency_list.begin(), latency_list.end());
                    float p95latency = latency_list[latency_list.size() * 0.95];
                    float p99latency = latency_list[latency_list.size() * 0.99];
                    std::vector<float> recalls(query_num);
                    for (unsigned i = 0; i < query_num; i++)
                    {
                        int correct = 0;
                        for (unsigned j = 0; j < K; j++)
                        {
                            for (unsigned g = 0; g < K; g++)
                            {
                                // size_t external_id = engine.internal_to_external[set_K_list[i][j]];
                                if (set_K_list[i][j] == true_nn_list[i][g])
                                {
                                    correct++;
                                    break;
                                }
                            }
                        }
                        recalls[i] = (float)correct / K;
                    }
                    float accumulate_recall = std::accumulate(recalls.begin(), recalls.end(), 0.0f);
                    float avg_recall = accumulate_recall / recalls.size();
                    std::sort(recalls.begin(), recalls.end());
                    float p95recall = recalls[recalls.size() * 0.05];
                    float p99recall = recalls[recalls.size() * 0.01];
                    { // Basic output
                        printf("%u,%u,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
                               L_master,
                               subsearch_iterations,
                               query_num / diff.count(),
                               avg_latency,
                               avg_recall,
                               p95recall,
                               p99recall,
                               p95latency,
                               p99latency,
                               (float)engine.count_distance_computation_ / query_num,
                               (float)engine.max_distance_computation_ / query_num,
                               (float)engine.count_hops_ / (query_num * num_threads),
                               engine.count_merge_ * 1.0 / query_num,
                               engine.time_expand_,
                               engine.time_merge_,
                               engine.time_seq_,
                               engine.time_expand_ / diff.count() * 100.0,
                               engine.time_merge_ / diff.count() * 100.0,
                               engine.time_seq_ / diff.count() * 100.0);
                    }
                    engine.count_distance_computation_ = 0;
                    engine.max_distance_computation_ = 0;
                    engine.count_hops_ = 0;
                    engine.count_merge_ = 0;
                    engine.time_expand_ = 0.0;
                    engine.time_merge_ = 0.0;
                    engine.time_seq_ = 0.0;
                }
                //                } // Index_threshold Ranged
            } // X Ranged
            //        } // L_local ranged
        } // L_master ranged
    }

    return 0;
}
