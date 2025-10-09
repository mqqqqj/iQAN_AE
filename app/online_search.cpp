//
// Created by Zhen Peng on 02/14/2021.
//

#include <iostream>
#include <cstdio>
#include <vector>
#include <chrono>
#include <clocale>
#include <omp.h>
#include <thread>
#include "../core/Searching.202102022027.PSS_v5.dist_thresh.profiling.h"
#include <queue>              // 新增：引入队列
#include <mutex>              // 新增：引入互斥锁
#include <condition_variable> // 新增：引入条件变量
#include <sys/resource.h>
#include <sstream>

void usage(int argc, char *argv[])
{
    if (argc != 10)
    {
        fprintf(stderr,
                "Usage: %s <data_file> <query_file> <nsg_path> "
                "<K> <result_file> <true_NN_file> <num_threads> "
                "<L> <request_rate>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
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
    const unsigned K = strtoull(argv[4], nullptr, 0);
    std::cout << "K: " << K << "\n";
    const char *path_results = argv[5];
    std::vector<std::vector<PANNS::idi>> true_nn_list; // groundtruth
    engine.load_groundtruth(
        argv[6],
        true_nn_list);
    unsigned data_dimension = engine.dimension_;
    unsigned points_num = engine.num_v_;
    if (engine.num_queries_ > 1000)
    {
        std::cout << "only use first 1k query" << std::endl;
        engine.num_queries_ = 1000;
    }
    unsigned query_num = engine.num_queries_;

    int num_threads = strtoull(argv[7], nullptr, 0);
    engine.num_threads_ = num_threads;
    omp_set_num_threads(num_threads);
    std::vector<int> L_list;
    std::string L_str = argv[8];
    std::stringstream ss(L_str);
    std::string L_val;
    while (std::getline(ss, L_val, ','))
    {
        L_list.push_back(std::stoi(L_val));
    }
    float request_rate = strtof(argv[9], nullptr);
    // printf("L,Throughput,latency,recall,total_dist_comps,max_dist_comps,hops,avg_merge,t_expand(s.),t_merge(s.),t_seq(s.),t_p_expand(%%),t_p_merge(%%),t_p_seq(%%)\n");
    std::cout << "L,Throughput,latency,recall" << std::endl;
    for (int L : L_list)
    {
        const unsigned L_master_low = L;
        const unsigned L_master_up = L;
        const unsigned L_master_step = 8;
        const unsigned L_local_low = 0;
        const unsigned L_local_up = 0;
        const unsigned L_local_step = 0;
        const unsigned X_low = L;
        const unsigned X_up = L;
        const unsigned X_step = 8;
        const unsigned I_thresh_low = 0;
        const unsigned I_thresh_up = 0;
        const unsigned I_thresh_step = 0;

        for (unsigned L_master = L_master_low; L_master <= L_master_up; L_master += L_master_step)
        {
            unsigned L_local = L_master;
            unsigned Index_thresh = L_local - 1;
            for (unsigned subsearch_iterations = X_low; subsearch_iterations <= X_up; subsearch_iterations += X_step)
            {
                engine.index_thresh_ = Index_thresh;
                unsigned warmup_max = 1;

                for (unsigned warmup = 0; warmup < warmup_max; ++warmup)
                {
                    // 新增：请求队列和相关同步工具
                    std::deque<unsigned> request_queue; // 使用 std::deque 替代 std::queue
                    std::mutex queue_mutex;
                    std::condition_variable queue_cv;
                    bool done = false;
                    std::vector<std::chrono::high_resolution_clock::time_point> query_receive_time(query_num);
                    std::vector<std::chrono::high_resolution_clock::time_point> query_search_start_time(query_num);
                    std::vector<std::chrono::high_resolution_clock::time_point> query_search_end_time(query_num);
                    unsigned nq = 1; // 可根据需求调整batchsize
                    std::thread request_generator([&]()
                                                  {
                unsigned i = 0;
                while (i < query_num)
                {
                    std::vector<unsigned> batch;
                    for (unsigned j = 0; j < nq && i < query_num; ++j, ++i)
                    {
                        batch.push_back(i);
                    }
        
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        for (auto &item : batch)
                        {
                            request_queue.push_back(item); // 使用 push_back
                            query_receive_time[item] = std::chrono::high_resolution_clock::now();
                        }
                        queue_cv.notify_all(); // 通知所有等待线程有新任务
                    }
        
                    if (i < query_num)
                    {
                        std::this_thread::sleep_for(std::chrono::duration<float, std::milli>(1000.0 / request_rate));
                    }
                }
        
                std::lock_guard<std::mutex> lock(queue_mutex);
                done = true;
                queue_cv.notify_all(); });

                    std::vector<std::vector<PANNS::idi>> set_K_list(query_num); // result set
                    for (unsigned i = 0; i < query_num; i++)
                        set_K_list[i].resize(K);
                    std::vector<PANNS::idi> init_ids(L_master);
                    boost::dynamic_bitset<> is_visited(points_num);
                    std::vector<PANNS::Candidate> set_L((num_threads - 1) * L_local + L_master); // all threads using a single set_L  (retset in nsg)
                    std::vector<PANNS::idi> local_queues_sizes(num_threads, 0);                  // initialize with all zeros
                    std::vector<PANNS::idi> local_queues_starts(num_threads);
                    for (int q_i = 0; q_i < num_threads; ++q_i)
                    {
                        local_queues_starts[q_i] = q_i * L_local;
                    }
                    std::vector<float> latency_list(query_num); // 单位：毫秒
                    std::vector<float> queueing_time_list(query_num);
                    std::vector<float> processing_time_list(query_num);
                    auto s = std::chrono::high_resolution_clock::now();
                    engine.prepare_init_ids(init_ids, L_master);
                    while (true)
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        queue_cv.wait(lock, [&]()
                                      { return !request_queue.empty() || done; });
                        if (request_queue.empty() && done)
                            break;
                        auto query_id = request_queue.front(); // 使用 front
                        request_queue.pop_front();             // 使用 pop_front
                        lock.unlock();
                        query_search_start_time[query_id] = std::chrono::high_resolution_clock::now();
                        engine.para_search_PSS_v5_dist_thresh_profiling(
                            query_id,
                            K,
                            L_master,
                            set_L,
                            init_ids,
                            set_K_list[query_id],
                            L_local,
                            local_queues_starts,
                            local_queues_sizes,
                            is_visited,
                            subsearch_iterations);
                        query_search_end_time[query_id] = std::chrono::high_resolution_clock::now();
                        latency_list[query_id] = std::chrono::duration_cast<std::chrono::microseconds>(
                                                     query_search_end_time[query_id] - query_receive_time[query_id])
                                                     .count() /
                                                 1000.0f; // 转换为毫秒
                        processing_time_list[query_id] = std::chrono::duration_cast<std::chrono::microseconds>(
                                                             query_search_end_time[query_id] - query_search_start_time[query_id])
                                                             .count() /
                                                         1000.0f; // 转换为毫秒
                        queueing_time_list[query_id] = std::chrono::duration_cast<std::chrono::microseconds>(
                                                           query_search_start_time[query_id] - query_receive_time[query_id])
                                                           .count() /
                                                       1000.0f; // 转换为毫秒
                        // if (query_id % 200 == 199)
                        //     std::cout << query_id + 1 << " done" << std::endl;
                    }
                    auto e = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> diff = e - s;
                    // std::cout << "Throughput(QPS): " << query_num / diff.count() << "\n";
                    // 计算平均latency
                    float avg_latency = std::accumulate(latency_list.begin(), latency_list.end(), 0.0f) / latency_list.size();
                    // std::cout << "avg_latency (actual latency): " << avg_latency << " ms" << std::endl;
                    // std::cout << "avg queueing time: " << std::accumulate(queueing_time_list.begin(), queueing_time_list.end(), 0.0f) / queueing_time_list.size() << "ms" << std::endl;
                    // std::cout << "avg processing time (ideal latency): " << std::accumulate(processing_time_list.begin(), processing_time_list.end(), 0.0f) / processing_time_list.size() << "ms" << std::endl;
                    std::unordered_map<unsigned, double> recalls;
                    { // Recall values
                        engine.get_recall_for_all_queries(
                            true_nn_list,
                            set_K_list,
                            recalls,
                            K);
                    }
                    { // Basic output

                        std::cout << L_master << "," << query_num / diff.count() << "," << avg_latency << "," << recalls[K] << std::endl;
                    }
                    engine.count_distance_computation_ = 0;
                    //                engine.count_iterations_ = 0;
                    //                    engine.count_checked_ = 0;
                    engine.count_merge_ = 0;
                    engine.time_expand_ = 0.0;
                    engine.time_merge_ = 0.0;
                    engine.time_seq_ = 0.0;
                    //                engine.time_pick_ = 0.0;
                    //                PANNS::DiskIO::save_result(path_results, set_K_list);
                    request_generator.join();
                }
                //                } // Index_threshold Ranged
            } // X Ranged
            //        } // L_local ranged
        } // L_master ranged
    }

    return 0;
}
