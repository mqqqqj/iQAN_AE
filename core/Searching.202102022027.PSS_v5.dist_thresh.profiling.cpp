//
// Created by Zhen Peng on 02/02/2021.
//

#include "Searching.202102022027.PSS_v5.dist_thresh.profiling.h"
#define BREAKDOWN_PRINT

namespace PANNS
{
    void Searching::load_groundtruth(const char *filename,
                                     std::vector<std::vector<idi>> &true_nn_list)
    {
        // data format must be : nq(unsigned), gk(unsigned), ids(nq*gk), distances(nq*gk)
        std::ifstream in(filename, std::ios::binary);
        if (!in.is_open())
        {
            std::cout << "open file error" << std::endl;
            exit(-1);
        }
        unsigned GK, nq;
        in.read((char *)&nq, sizeof(unsigned));
        in.read((char *)&GK, sizeof(unsigned));
        for (unsigned i = 0; i < nq; i++)
        {
            std::vector<unsigned> result(GK);
            in.read((char *)result.data(), GK * sizeof(unsigned));
            true_nn_list.push_back(result);
            groundtruth.push_back(result);
        }
        in.close();
    }
    /**
     * Input the data from the file.
     * @param filename
     */
    void Searching::load_data_load(char *filename)
    {
        auto old_d = dimension_;
        DiskIO::load_fbin(
            filename,
            data_load_,
            num_v_,
            dimension_);
        if (old_d)
        {
            if (old_d != dimension_)
            {
                std::cerr << "Error: data dimension " << dimension_
                          << " is not equal to query dimension " << old_d << "." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    /**
     * Input queries from the file.
     * @param filename
     */
    void Searching::load_queries_load(char *filename)
    {
        auto old_d = dimension_;
        DiskIO::load_fbin(
            filename,
            queries_load_,
            num_queries_,
            dimension_);
        if (old_d)
        {
            if (old_d != dimension_)
            {
                std::cerr << "Error: query dimension " << dimension_
                          << " is not equal to data dimension " << old_d << "." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    /**
     * Input the NSG graph from the file.
     * Reference: https://github.com/ZJULearning/nsg/blob/master/src/index_nsg.cpp
     * @param filename
     */
    void Searching::load_nsg_graph(char *filename)
    {
        std::ifstream fin(filename);
        if (!fin.is_open())
        {
            std::cerr << "Error: cannot read file " << filename << " ." << std::endl;
            exit(EXIT_FAILURE);
        }
        fin.read(reinterpret_cast<char *>(&width_), sizeof(unsigned));
        fin.read(reinterpret_cast<char *>(&ep_), sizeof(unsigned));

        data_bytes_ = (1 + dimension_) * sizeof(dataf);
        neighbor_bytes_ = (1 + width_) * sizeof(idi);
        vertex_bytes_ = data_bytes_ + neighbor_bytes_;
        opt_nsg_graph_ = (char *)malloc(num_v_ * vertex_bytes_);
        if (!opt_nsg_graph_)
        {
            std::cerr << "Error: no enough memory for opt_nsg_graph_." << std::endl;
            exit(EXIT_FAILURE);
        }

        idi v_id = 0;
        num_e_ = 0;
        char *base_location = opt_nsg_graph_;
        while (true)
        {
            idi degree;
            fin.read(reinterpret_cast<char *>(&degree), sizeof(unsigned));
            if (fin.eof())
            {
                break;
            }
            num_e_ += degree;
            //        std::vector<idi> tmp_ngbrs(degree);
            //        fin.read(reinterpret_cast<char *>(tmp_ngbrs.data()), degree * sizeof(unsigned));

            // Norm and data
            distf norm = compute_norm(data_load_ + v_id * dimension_);
            //        distf norm = compute_norm(v_id);
            std::memcpy(base_location, &norm, sizeof(distf));                                                  // Norm
            memcpy(base_location + sizeof(distf), data_load_ + v_id * dimension_, dimension_ * sizeof(dataf)); // Data
            base_location += data_bytes_;

            // Neighbors
            memcpy(base_location, &degree, sizeof(idi));                      // Number of neighbors
            fin.read(base_location + sizeof(idi), degree * sizeof(unsigned)); // Neighbors
                                                                              //        memcpy(location + sizeof(idi), tmp_ngbrs.data(), degree * sizeof(unsigned));
            base_location += neighbor_bytes_;
            ++v_id;
        }
        if (v_id != num_v_)
        {
            std::cerr << "Error: NSG data has " << v_id
                      << " vertices, but origin data has " << num_v_ << " vertices." << std::endl;
            exit(EXIT_FAILURE);
        }
        free(data_load_);
        data_load_ = nullptr;
        //    ////////////////////////
        //    idi v_id = 0;
        //    num_e_ = 0;
        //    while (true) {
        //        idi degree;
        //        fin.read(reinterpret_cast<char *>(&degree), sizeof(unsigned));
        //        if (fin.eof()) {
        //            break;
        //        }
        //        num_e_ += degree;
        //
        //        std::vector<idi> ngbrs(degree);
        //        fin.read(reinterpret_cast<char *>(ngbrs.data()), degree * sizeof(unsigned));
        ////        nsg_graph_.push_back(ngbrs);
        ////        tmp_edge_list.push_back(ngbrs);
        //        edge_list_.push_back(ngbrs);
        //        ++v_id;
        //    }
        //    if (v_id != num_v_) {
        //        std::cerr << "Error: NSG data has " << v_id
        //                  << " vertices, but origin data has " << num_v_ << " vertices." << std::endl;
        //        exit(EXIT_FAILURE);
        //    }
    }

    /**
     * Input the HNSW graph from the file.
     * @param filename
     */
    void Searching::load_hnsw_graph(char *hnswfilename)
    {
        using std::size_t;

        // 1. 打开 HNSW 索引（二进制）
        std::ifstream in(hnswfilename, std::ios::binary);
        if (!in.is_open())
        {
            std::cerr << "Error: cannot read HNSW file " << hnswfilename << " ." << std::endl;
            exit(EXIT_FAILURE);
        }

        // 2. 按 hnswlib::saveIndex 的顺序读取头部
        size_t offsetLevel0 = 0;
        size_t max_elements = 0;
        size_t cur_element_count = 0;
        size_t size_data_per_element = 0;
        size_t label_offset = 0;
        size_t offsetData = 0;
        int maxlevel = 0;
        unsigned enterpoint_node = 0; // tableint = unsigned int
        size_t maxM = 0;
        size_t maxM0 = 0;
        size_t M = 0;
        double mult = 0.0;
        size_t ef_construction = 0;

        auto readPOD = [&](auto &x)
        {
            in.read(reinterpret_cast<char *>(&x), sizeof(x));
            if (!in)
            {
                std::cerr << "Error: failed to read HNSW header." << std::endl;
                exit(EXIT_FAILURE);
            }
        };

        readPOD(offsetLevel0);
        readPOD(max_elements);
        readPOD(cur_element_count);
        readPOD(size_data_per_element);
        readPOD(label_offset);
        readPOD(offsetData);
        readPOD(maxlevel);
        readPOD(enterpoint_node);
        readPOD(maxM);
        readPOD(maxM0);
        readPOD(M);
        readPOD(mult);
        readPOD(ef_construction);

        // 3. 读取 base layer 整块内存：data_level0_memory_
        const size_t total_bytes = cur_element_count * size_data_per_element;
        std::vector<char> base_mem(total_bytes);
        in.read(base_mem.data(), total_bytes);
        if (!in)
        {
            std::cerr << "Error: failed to read HNSW base layer." << std::endl;
            exit(EXIT_FAILURE);
        }

        // 4. 从 HNSW 推导向量维度 dim_from_hnsw，并与当前 dimension_ 对齐
        size_t data_size = label_offset - offsetData; // 每个点的向量字节数
        if (data_size % sizeof(dataf) != 0)
        {
            std::cerr << "Error: HNSW data_size is not multiple of sizeof(dataf)." << std::endl;
            exit(EXIT_FAILURE);
        }
        size_t dim_from_hnsw = data_size / sizeof(dataf);

        if (dimension_ == 0)
        {
            // 如果框架里 dimension_ 还没设置，就直接用 HNSW 的
            dimension_ = static_cast<unsigned>(dim_from_hnsw);
        }
        else if (dimension_ != static_cast<unsigned>(dim_from_hnsw))
        {
            std::cerr << "Error: dimension mismatch. HNSW dim = "
                      << dim_from_hnsw << ", framework dim = " << dimension_ << std::endl;
            exit(EXIT_FAILURE);
        }

        // 5. 设置点数、度上限、入口点
        num_v_ = static_cast<idi>(cur_element_count);
        ep_ = static_cast<idi>(enterpoint_node);

        // HNSW 的 level0 最大度是 maxM0_，用它作为 width_
        width_ = static_cast<unsigned>(maxM0);

        // 6. 根据新 width_ 和 dimension_ 重新计算布局，并分配 opt_nsg_graph_
        data_bytes_ = (1 + dimension_) * sizeof(dataf); // norm + 向量
        neighbor_bytes_ = (1 + width_) * sizeof(idi);   // degree + 邻居列表
        vertex_bytes_ = data_bytes_ + neighbor_bytes_;

        if (opt_nsg_graph_)
        {
            free(opt_nsg_graph_);
            opt_nsg_graph_ = nullptr;
        }

        opt_nsg_graph_ = (char *)malloc(static_cast<size_t>(num_v_) * vertex_bytes_);
        if (!opt_nsg_graph_)
        {
            std::cerr << "Error: no enough memory for opt_nsg_graph_." << std::endl;
            exit(EXIT_FAILURE);
        }

        // 7. 如果原来有 data_load_，可以释放掉；后面完全用 HNSW 里的向量
        if (data_load_)
        {
            free(data_load_);
            data_load_ = nullptr;
        }

        // 8. 准备 internal->external id 映射
        internal_to_external.clear();
        internal_to_external.resize(cur_element_count);

        // 9. 遍历每个 internal id，写入 opt_nsg_graph_，并填映射表
        num_e_ = 0;
        char *base_location = opt_nsg_graph_;

        for (size_t i = 0; i < cur_element_count; ++i)
        {
            char *base_i = base_mem.data() + i * size_data_per_element;

            // 9.1 取出向量数据（HNSW 的 getDataByInternalId）
            dataf *vec_ptr = reinterpret_cast<dataf *>(base_i + offsetData);

            // 先写 norm + 向量到 opt_nsg_graph_
            // Norm
            distf norm = compute_norm(vec_ptr); // 保证 compute_norm 支持 dataf* 输入
            std::memcpy(base_location, &norm, sizeof(distf));

            // Data
            std::memcpy(base_location + sizeof(distf),
                        vec_ptr,
                        static_cast<size_t>(dimension_) * sizeof(dataf));

            base_location += data_bytes_;

            // 9.2 解析 level0 邻接表（HNSW 的 get_linklist0）
            char *ll0_ptr = base_i + offsetLevel0;

            // 注意：linklistsizeint 在 hnsw 里是 unsigned int，
            // getListCount 实际返回的是 unsigned short，前 2 字节是邻居数
            unsigned short degree = *reinterpret_cast<unsigned short *>(ll0_ptr);

            // 邻居 id 数组紧跟在 linklistsizeint 后面（sizeof(unsigned int)）
            unsigned *neighbors =
                reinterpret_cast<unsigned *>(ll0_ptr + sizeof(unsigned)); // sizeof(linklistsizeint)

            // 统计边数
            num_e_ += degree;

            // 9.3 写邻居到 opt_nsg_graph_（格式：degree + degree 个邻居，剩余空间随意）
            idi deg_i = static_cast<idi>(degree);
            std::memcpy(base_location, &deg_i, sizeof(idi)); // 邻居数量

            // 写前 degree 个邻居（内部 id）
            std::memcpy(base_location + sizeof(idi),
                        neighbors,
                        static_cast<size_t>(degree) * sizeof(idi));

            // 如果你想把后面的空间清零（可选）
            // size_t remain = (width_ - degree) * sizeof(idi);
            // if (remain > 0) {
            //     std::memset(base_location + sizeof(idi) + degree * sizeof(idi), 0, remain);
            // }

            base_location += neighbor_bytes_;

            // 9.4 解析 external label，填映射表
            size_t *label_ptr = reinterpret_cast<size_t *>(base_i + label_offset);
            internal_to_external[i] = *label_ptr;
        }

        // 10. 简单检查
        if (base_location != opt_nsg_graph_ + static_cast<size_t>(num_v_) * vertex_bytes_)
        {
            std::cerr << "Error: written bytes mismatch in opt_nsg_graph_." << std::endl;
            exit(EXIT_FAILURE);
        }
        free(data_load_);
        data_load_ = nullptr;
    }

    /**
     * Load those true top-K neighbors (ground truth) of queries
     * @param filename
     * @param[out] true_nn_list
     */
    void Searching::load_true_NN(
        const char *filename,
        std::vector<std::vector<idi>> &true_nn_list)
    //        unsigned &t_K)
    {
        std::ifstream fin(filename);
        if (!fin.is_open())
        {
            fprintf(stderr, "Error: cannot open file %s\n", filename);
            exit(EXIT_FAILURE);
        }
        idi t_query_num;
        idi t_K;
        //    unsigned t_K;
        fin.read(reinterpret_cast<char *>(&t_query_num), sizeof(t_query_num));
        fin.read(reinterpret_cast<char *>(&t_K), sizeof(t_K));
        //    if (t_query_num != query_num) {
        //        fprintf(stderr, "Error: query_num %u is not equal to the record %u in true-NN file %s\n",
        //                query_num, t_query_num, filename);
        //        exit(EXIT_FAILURE);
        //    }
        if (t_query_num < num_queries_)
        {
            fprintf(stderr, "Error: t_query_num %u is smaller than num_queries_ %u\n", t_query_num, num_queries_);
            exit(EXIT_FAILURE);
        }
        if (t_K < 100)
        {
            fprintf(stderr, "Error: t_K %u is smaller than 100.\n", t_K);
            exit(EXIT_FAILURE);
        }

        //    data = new unsigned[(size_t) t_query_num * (size_t) t_K];
        true_nn_list.resize(t_query_num);
        for (idi q_i = 0; q_i < t_query_num; ++q_i)
        {
            true_nn_list[q_i].resize(t_K);
        }

        for (unsigned q_i = 0; q_i < t_query_num; ++q_i)
        {
            //        size_t offset = q_i * t_K;
            for (unsigned n_i = 0; n_i < t_K; ++n_i)
            {
                unsigned id;
                float dist;
                fin.read(reinterpret_cast<char *>(&id), sizeof(id));
                fin.read(reinterpret_cast<char *>(&dist), sizeof(dist));
                //            data[offset + n_i] = id;
                true_nn_list[q_i][n_i] = id;
            }
        }

        fin.close();
    }
    void Searching::get_recall_for_all_queries(
        const std::vector<std::vector<idi>> &true_nn_list,
        const std::vector<std::vector<unsigned>> &set_K_list,
        std::unordered_map<unsigned, double> &recalls,
        const idi L) const
    {
        if (true_nn_list[0].size() < 100)
        {
            fprintf(stderr, "Error: Number of true nearest neighbors of a query is smaller than 100.\n");
            exit(EXIT_FAILURE);
        }
        recalls[1] = 0.0;
        recalls[5] = 0.0;
        recalls[10] = 0.0;
        recalls[20] = 0.0;
        recalls[50] = 0.0;
        recalls[100] = 0.0;

        const idi set_K_size = L < 100 ? L : 100;

        for (unsigned q_i = 0; q_i < num_queries_; ++q_i)
        {
            // 对于每个查询，计算不同K值的recall
            for (unsigned K : {1, 5, 10, 20, 50, 100})
            {
                if (K > set_K_size)
                    continue; // 如果K大于返回结果数量，跳过

                // 计算在前K个返回结果中有多少个是真实的前K个最近邻
                unsigned correct_count = 0;
                for (unsigned i = 0; i < K; ++i)
                {
                    unsigned result_id = set_K_list[q_i][i];
                    // 检查这个结果是否在真实的前K个最近邻中
                    for (unsigned j = 0; j < K; ++j)
                    {
                        if (result_id == true_nn_list[q_i][j])
                        {
                            correct_count++;
                            break;
                        }
                    }
                }
                recalls[K] += (double)correct_count / K;
            }
        }

        // 计算平均recall
        for (unsigned K : {1, 5, 10, 20, 50, 100})
        {
            recalls[K] /= num_queries_;
        }
    }

    /**
     * Prepare init_ids and flags, as they are constant for all queries.
     * @param[out] init_ids
     * @param L
     */
    void Searching::prepare_init_ids(
        std::vector<unsigned int> &init_ids,
        const unsigned L) const
    {
        //    idi num_ngbrs = get_out_degree(ep_);
        //    edgei edge_start = nsg_graph_indices_[ep_];
        //    // Store ep_'s neighbors as candidates
        //    idi tmp_l = 0;
        //    for (; tmp_l < L && tmp_l < num_ngbrs; tmp_l++) {
        //        init_ids[tmp_l] = nsg_graph_out_edges_[edge_start + tmp_l];
        //    }
        //    std::unordered_set<idi> visited_ids;
        boost::dynamic_bitset<> is_selected(num_v_);
        idi *out_edges = (idi *)(opt_nsg_graph_ + ep_ * vertex_bytes_ + data_bytes_);
        idi out_degree = *out_edges++;
        idi init_ids_end = 0;
        //    for (; tmp_l < L && tmp_l < out_degree; tmp_l++) {
        for (idi e_i = 0; e_i < out_degree && init_ids_end < L; ++e_i)
        {
            //        idi v_id = out_edges[tmp_l];
            idi v_id = out_edges[e_i];
            if (is_selected[v_id])
            {
                continue;
            }
            is_selected[v_id] = true;
            //        init_ids[tmp_l] = v_id;
            init_ids[init_ids_end++] = v_id;
            //        init_ids[tmp_l] = out_edges[tmp_l];
            //        visited_ids.insert(init_ids[tmp_l]);
        }

        //    for (idi i = 0; i < tmp_l; ++i) {
        //        is_visited[init_ids[i]] = true;
        //    }

        // If ep_'s neighbors are not enough, add other random vertices
        idi tmp_id = ep_ + 1; // use tmp_id to replace rand().
        while (init_ids_end < L)
        {
            //        tmp_id %= num_v_;
            if (tmp_id == num_v_)
            {
                tmp_id = 0;
            }
            idi v_id = tmp_id++;
            if (is_selected[v_id])
            {
                continue;
            }
            //        if (visited_ids.find(id) != visited_ids.end()) {
            //            continue;
            //        }
            is_selected[v_id] = true;
            //        visited_ids.insert(id);
            init_ids[init_ids_end++] = v_id;
            //        tmp_l++;
        }
    }

    // TODO: re-code in AVX-512
    dataf Searching::compute_norm(
        const dataf *data) const
    //        idi vertex_id)
    //        const std::vector<PANNS::dataf> &data)
    //        size_t loc_start,
    //        idi dimension)
    {
        //    const dataf *a = data.data() + loc_start;
        //    const dataf *a = data_load_ + vertex_id * dimension_;
        //    idi size = dimension_;
        dataf result = 0;
// #define AVX_L2NORM(addr, dest, tmp) \
//     tmp = _mm256_load_ps(addr); \
//     tmp = _mm256_mul_ps(tmp, tmp); \
//     dest = _mm256_add_ps(dest, tmp);
#define AVX_L2NORM(addr, dest, tmp) \
    tmp = _mm256_loadu_ps(addr);    \
    tmp = _mm256_mul_ps(tmp, tmp);  \
    dest = _mm256_add_ps(dest, tmp);

        __m256 sum;
        __m256 l0, l1;
        unsigned D = (dimension_ + 7) & ~7U;
        unsigned DR = D % 16;
        unsigned DD = D - DR;
        const float *l = data;
        const float *e_l = l + DD;
        float unpack[8] __attribute__((aligned(32))) = {0, 0, 0, 0, 0, 0, 0, 0};

        sum = _mm256_load_ps(unpack);
        //    sum = _mm256_loadu_ps(unpack);
        if (DR)
        {
            AVX_L2NORM(e_l, sum, l0);
        }
        for (unsigned i = 0; i < DD; i += 16, l += 16)
        {
            AVX_L2NORM(l, sum, l0);
            AVX_L2NORM(l + 8, sum, l1);
        }
        _mm256_store_ps(unpack, sum);
        //    _mm256_storeu_ps(unpack, sum);
        result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];

        return result;
    }
    // inner product
    dataf Searching::compute_distance_with_norm(
        const dataf *v_data,
        const dataf *q_data,
        const dataf vertex_norm) const
    {
        float result = 0;
#define AVX_DOT(addr1, addr2, dest, tmp1, tmp2) \
    tmp1 = _mm256_loadu_ps(addr1);              \
    tmp2 = _mm256_loadu_ps(addr2);              \
    tmp1 = _mm256_mul_ps(tmp1, tmp2);           \
    dest = _mm256_add_ps(dest, tmp1);
        __m256 sum;
        __m256 l0, l1;
        __m256 r0, r1;
        unsigned D = (dimension_ + 7) & ~7U;
        unsigned DR = D % 16;
        unsigned DD = D - DR;
        const float *l = v_data;
        const float *r = q_data;
        //    const float *l = (float *) (opt_nsg_graph_ + vertex_id * vertex_bytes_ + sizeof(distf));
        //    const float *r = queries_load_ + query_id * dimension_;
        const float *e_l = l + DD;
        const float *e_r = r + DD;
        float unpack[8] __attribute__((aligned(32))) = {0, 0, 0, 0, 0, 0, 0, 0};
        sum = _mm256_load_ps(unpack);
        //    sum = _mm256_loadu_ps(unpack);
        if (DR)
        {
            AVX_DOT(e_l, e_r, sum, l0, r0);
        }

        for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16)
        {
            AVX_DOT(l, r, sum, l0, r0);
            AVX_DOT(l + 8, r + 8, sum, l1, r1);
        }
        _mm256_store_ps(unpack, sum);
        //    _mm256_storeu_ps(unpack, sum);
        result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];
        // result = -2 * result + vertex_norm;
        result = -result;
        return result;
    }
    // L2
    //     dataf Searching::compute_distance_with_norm(
    //         const dataf *v_data,
    //         const dataf *q_data,
    //         const dataf vertex_norm) const
    //     {
    //         float result = 0;
    // #define AVX_L2SQR(addr1, addr2, dest, tmp1, tmp2) \
    //     tmp1 = _mm256_loadu_ps(addr1);                \
    //     tmp2 = _mm256_loadu_ps(addr2);                \
    //     tmp1 = _mm256_sub_ps(tmp1, tmp2);             \
    //     tmp1 = _mm256_mul_ps(tmp1, tmp1);             \
    //     dest = _mm256_add_ps(dest, tmp1);

    //         __m256 sum;
    //         __m256 l0, l1;
    //         __m256 r0, r1;
    //         unsigned D = (dimension_ + 7) & ~7U;
    //         unsigned DR = D % 16;
    //         unsigned DD = D - DR;
    //         const float *l = v_data;
    //         const float *r = q_data;
    //         const float *e_l = l + DD;
    //         const float *e_r = r + DD;
    //         float unpack[8] __attribute__((aligned(32))) = {0, 0, 0, 0, 0, 0, 0, 0};

    //         sum = _mm256_loadu_ps(unpack);
    //         if (DR)
    //         {
    //             AVX_L2SQR(e_l, e_r, sum, l0, r0);
    //         }

    //         for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16)
    //         {
    //             AVX_L2SQR(l, r, sum, l0, r0);
    //             AVX_L2SQR(l + 8, r + 8, sum, l1, r1);
    //         }
    //         _mm256_store_ps(unpack, sum);
    //         result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];
    //         return result;
    //     }

    //
    // The difference from insert_into_queue is that add_into_queue will increase the queue size by 1.
    // add_into_queue with a queue_start
    idi Searching::add_into_queue(
        std::vector<PANNS::Candidate> &queue,
        const idi queue_start,
        idi &queue_size,          // The insertion location starting from queue_start
        const idi queue_capacity, // The maximum capacity of queue, independent with queue_start.
        const PANNS::Candidate &cand)
    {
        if (0 == queue_size)
        {
            queue[queue_start + queue_size++] = cand;
            return 0;
        }
        idi queue_end = queue_start + queue_size;
        // Find the insert location
        const auto it_loc = std::lower_bound(queue.begin() + queue_start, queue.begin() + queue_end, cand);
        //    auto it_loc = std::lower_bound(queue.begin(), queue.begin() + queue_size, cand);
        idi insert_loc = it_loc - queue.begin();

        if (insert_loc != queue_end)
        {
            if (cand.id_ == it_loc->id_)
            {
                // Duplicate
                return queue_capacity;
            }
            if (queue_size >= queue_capacity)
            { // Queue is full
                --queue_size;
                --queue_end;
            }
        }
        else
        { // insert_loc == queue_end, insert at the end?
            if (queue_size < queue_capacity)
            { // Queue is not full
                // Insert at the end
                queue[insert_loc] = cand;
                ++queue_size;
                return queue_size - 1;
            }
            else
            { // Queue is full
                return queue_capacity;
            }
        }
        // Add into queue
        memmove(reinterpret_cast<char *>(queue.data() + insert_loc + 1),
                reinterpret_cast<char *>(queue.data() + insert_loc),
                (queue_end - insert_loc) * sizeof(Candidate));
        queue[insert_loc] = cand;
        ++queue_size;
        return insert_loc - queue_start;
    }

    void Searching::add_into_queue_at(
        const Candidate &cand,
        std::vector<Candidate> &queue,
        const idi insert_index, // The insertion location, independent with queue_start
        const idi queue_start,
        idi &queue_size,        // The number of elements in queue, independent with queue_start
        const idi queue_length) // The maximum capacity of queue, independent with queue_start.
    {
        const idi dest_index = queue_start + insert_index;
        if (queue_size == queue_length)
        {
            --queue_size;
        }
        memmove(reinterpret_cast<char *>(queue.data() + dest_index + 1),
                reinterpret_cast<char *>(queue.data() + dest_index),
                (queue_size - insert_index) * sizeof(Candidate));
        queue[dest_index] = cand;
        ++queue_size;
    }

    void Searching::insert_one_element_at(
        const Candidate &cand,
        std::vector<Candidate> &queue,
        const idi insert_index,
        const idi queue_start,
        const idi queue_size)
    {
        const idi dest_index = queue_start + insert_index;
        memmove(reinterpret_cast<char *>(queue.data() + dest_index + 1),
                reinterpret_cast<char *>(queue.data() + dest_index),
                (queue_size - insert_index - 1) * sizeof(Candidate));
        queue[dest_index] = cand;
    }

    /* Function:
     * queue1_size is fixed.
     */
    idi Searching::merge_two_queues_into_1st_queue_seq_fixed(
        std::vector<Candidate> &queue1,
        const idi queue1_start,
        const idi queue1_size,
        std::vector<Candidate> &queue2,
        const idi queue2_start,
        const idi queue2_size)
    //        const idi limit_size)
    {
        assert(queue1_size && queue2_size);
        // Record the lowest insert location.
        auto it_loc = std::lower_bound(
            queue1.begin() + queue1_start,
            queue1.begin() + queue1_start + queue1_size,
            queue2[queue2_start]);
        idi insert_index = it_loc - (queue1.begin() + queue1_start);
        if (insert_index == queue1_size)
        {
            return insert_index;
        }
        else if (insert_index == queue1_size - 1)
        {
            queue1[queue1_start + insert_index] = queue2[queue2_start];
            return insert_index;
        }

        // Insert the 1st of queue2
        if (queue2[queue2_start].id_ != it_loc->id_)
        {
            // Not Duplicate
            insert_one_element_at(
                queue2[queue2_start],
                queue1,
                insert_index,
                queue1_start,
                queue1_size);
        }
        else if (!queue2[queue2_start].is_checked_ && it_loc->is_checked_)
        {
            it_loc->is_checked_ = false;
        }
        if (queue2_size == 1)
        {
            return insert_index;
        }

        // Insert
        idi q_i_1 = insert_index + 1 + queue1_start;
        idi q_i_2 = queue2_start + 1;
        const idi q_i_1_bound = queue1_start + queue1_size;
        const idi q_i_2_bound = queue2_start + queue2_size;
        //    const idi insert_i_bound = queue1_start + limit_size;
        for (idi insert_i = insert_index + 1; insert_i < queue1_size; ++insert_i)
        {
            if (q_i_1 >= q_i_1_bound || q_i_2 >= q_i_2_bound)
            {
                // queue1 or queue2 finished traverse. Rest o
                break;
            }
            else if (queue1[q_i_1] < queue2[q_i_2])
            {
                ++q_i_1;
            }
            else if (queue2[q_i_2] < queue1[q_i_1])
            {
                // Insert queue2[q_i_2] into queue1
                insert_one_element_at(
                    queue2[q_i_2++],
                    queue1,
                    insert_i,
                    queue1_start,
                    queue1_size);
                ++q_i_1;
            }
            else
            {
                // Duplicate
                if (!queue2[q_i_2].is_checked_ && queue1[q_i_1].is_checked_)
                {
                    queue1[q_i_1].is_checked_ = false;
                }
                ++q_i_2;
                ++q_i_1;
            }
        }

        return insert_index;
    }

    /* Function:
     * queue1_size should be updated.
     * queue1_length should be provided.
     */
    idi Searching::merge_two_queues_into_1st_queue_seq_incr(
        std::vector<Candidate> &queue1,
        const idi queue1_start,
        idi &queue1_size,        // The number of element in queue1, independent with queue1_start.
        const idi queue1_length, // The maximum capacity of queue1, independent with queue1_start.
        std::vector<Candidate> &queue2,
        const idi queue2_start,
        const idi queue2_size)
    //        const idi limit_size)
    {
        assert(queue1_size && queue2_size);
        // Record the lowest insert location.
        auto it_loc = std::lower_bound(
            queue1.begin() + queue1_start,
            queue1.begin() + queue1_start + queue1_size,
            queue2[queue2_start]);
        idi insert_index = it_loc - (queue1.begin() + queue1_start);
        if (insert_index == queue1_size)
        {
            idi copy_count = (queue1_size + queue2_size > queue1_length) ? queue1_length - queue1_size : queue2_size;
            memmove(queue1.data() + queue1_start + queue1_size,
                    queue2.data() + queue2_start,
                    copy_count * sizeof(Candidate));
            queue1_size += copy_count;
            return insert_index;
        }
        if (queue2[queue2_start].id_ != it_loc->id_)
        {
            // Not Duplicate
            add_into_queue_at(
                queue2[queue2_start],
                queue1,
                insert_index,
                queue1_start,
                queue1_size,
                queue1_length);
        }
        else if (!queue2[queue2_start].is_checked_ && it_loc->is_checked_)
        {
            it_loc->is_checked_ = false;
        }
        if (queue2_size == 1)
        {
            return insert_index;
        }

        // Insert
        idi q_i_1 = insert_index + 1 + queue1_start;
        idi q_i_2 = queue2_start + 1;
        idi q_i_1_bound = queue1_start + queue1_size; // When queue1_size is updated, so should be q_i_1_bound.
        const idi q_i_2_bound = queue2_start + queue2_size;
        //    idi insert_i;
        for (idi insert_i = insert_index + 1; insert_i < queue1_length; ++insert_i)
        {
            if (q_i_1 >= q_i_1_bound)
            {
                queue1_size += std::min(queue1_length - insert_i, q_i_2_bound - q_i_2);
                for (; insert_i < queue1_size; ++insert_i)
                {
                    queue1[queue1_start + insert_i] = queue2[q_i_2++];
                }
                break;
            }
            else if (q_i_2 >= q_i_2_bound)
            {
                break;
            }
            else if (queue1[q_i_1] < queue2[q_i_2])
            {
                ++q_i_1;
            }
            else if (queue2[q_i_2] < queue1[q_i_1])
            {
                add_into_queue_at(
                    queue2[q_i_2++],
                    queue1,
                    insert_i,
                    queue1_start,
                    queue1_size,
                    queue1_length);
                ++q_i_1;
                q_i_1_bound = queue1_start + queue1_size;
            }
            else
            {
                // Duplicate
                if (!queue2[q_i_2].is_checked_ && queue1[q_i_1].is_checked_)
                {
                    queue1[q_i_1].is_checked_ = false;
                }
                ++q_i_2;
                ++q_i_1;
            }
        }
        return insert_index;
    }

    /* Function:
     * Use large local_queues_array as a concatenation of all queues
     */
    idi Searching::merge_all_queues_to_master(
        std::vector<Candidate> &set_L,
        const std::vector<idi> &local_queues_starts,
        std::vector<idi> &local_queues_sizes,
        const idi local_queue_capacity,
        const idi L) const
    {
        idi nk = L;
        const idi master_start = local_queues_starts[num_threads_ - 1];
        //    idi &master_size = local_queues_sizes[num_threads_ - 1];
        const idi w_i_bound = num_threads_ - 1;
        for (idi w_i = 0; w_i < w_i_bound; ++w_i)
        {
            const idi w_start = local_queues_starts[w_i];
            idi &w_size = local_queues_sizes[w_i];
            if (0 == w_size)
            {
                continue;
            }
            idi r = merge_two_queues_into_1st_queue_seq_fixed(
                set_L,
                master_start,
                L,
                set_L,
                w_start,
                w_size);
            //        idi r = merge_two_queues_into_1st_queue_seq_incr(
            //                set_L,
            //                master_start,
            //                master_size,
            //                L,
            //                set_L,
            //                w_start,
            //                w_size);
            if (r < nk)
            {
                nk = r;
            }
            w_size = 0;
        }

        return nk;
    }
    //// Bakeup
    // idi Searching::merge_all_queues_to_master(
    //         std::vector<Candidate> &set_L,
    //         const std::vector<idi> &local_queues_starts,
    //         std::vector<idi> &local_queues_sizes,
    //         const idi local_queue_capacity,
    //         const idi L) const
    //{
    ////    const int num_queues = num_threads_;
    //    idi nk = L;
    //    int size = 1 << (static_cast<idi>(log2(num_threads_)));
    //    idi log2size = static_cast<idi>(log2(size));
    //    for (idi d = 0; d < log2size; ++d) {
    //        uint32_t by = 1 << (d + 1);
    // #pragma omp parallel for
    //        for (int i = 0; i < size; i += by) {
    //            const idi ai = i + (1 << (d + 1)) - 1; // i + 2^(d+1) - 1
    //            const idi a_start = local_queues_starts[ai];
    //            idi &a_size = local_queues_sizes[ai];
    ////            idi a_start = ai * local_queue_length;
    //            const idi bi = i + (1 << d) - 1; // i + 2^d - 1
    //            const idi b_start = local_queues_starts[bi];
    //            idi &b_size = local_queues_sizes[bi];
    ////            idi b_start = bi * local_queue_length;
    //            if (0 == b_size) {
    //                continue;
    //            }
    //            if (a_size == 0) {
    //                std::copy(set_L.begin() + b_start,
    //                          set_L.begin() + b_start + b_size,
    //                          set_L.begin() + a_start); // Copy bi to ai
    //                a_size = b_size;
    //                b_size = 0;
    //                continue;
    //            }
    //            if (ai != static_cast<idi>(num_threads_ - 1)) {
    //                merge_two_queues_into_1st_queue_seq_incr(
    //                        set_L,
    //                        a_start,
    //                        a_size,
    //                        local_queue_capacity,
    //                        set_L,
    //                        b_start,
    //                        b_size);
    //            } else {
    //                idi r = merge_two_queues_into_1st_queue_seq_fixed(
    //                        set_L,
    //                        a_start,
    //                        L,
    //                        set_L,
    //                        b_start,
    //                        b_size);
    //                if (r < nk) {
    //                    nk = r;
    //                }
    //            }
    //            b_size = 0;
    //        }
    //    }
    //    // Reset local_queues_sizes
    //    // Not do this for Collector Idea or Selecting Idea
    ////    std::fill(local_queues_sizes.begin(), local_queues_sizes.end() - 1, 0);
    ////    std::fill(local_queues_sizes.begin(), local_queues_sizes.end(), 0);
    //
    //    return nk;
    ////    return r;
    //}

    /*
     * Function: pick master queue's top-M unchecked elements to top_m_candidates.
     * Used by interval merge.
     */
    idi Searching::pick_top_m_to_workers(
        //        const idi M,
        std::vector<Candidate> &set_L,
        const std::vector<idi> &local_queues_starts,
        std::vector<idi> &local_queues_sizes,
        const idi local_queue_capacity,
        const idi k_uc) const
    {
        const idi last_queue_start = local_queues_starts[num_threads_ - 1];
        idi c_i_start = k_uc + last_queue_start;
        idi c_i_bound = last_queue_start + local_queues_sizes[num_threads_ - 1];
        idi top_m_count = 0;
        int dest_queue = 0;
        for (idi c_i = c_i_start; c_i < c_i_bound; ++c_i)
        {
            if (set_L[c_i].is_checked_)
            {
                continue;
            }
            ++top_m_count;
            if (num_threads_ - 1 != dest_queue)
            {
                set_L[local_queues_starts[dest_queue] + local_queues_sizes[dest_queue]++] = set_L[c_i];
                set_L[c_i].is_checked_ = true;
                if (local_queues_sizes[dest_queue] == local_queue_capacity)
                {
                    break;
                }
                ++dest_queue;
            }
            else
            {
                dest_queue = 0;
            }
        }
        return top_m_count;
    }

    /*
     * Function: pick top-M unchecked candidates from queue
     */
    void Searching::pick_top_m_unchecked(
        const idi M,
        const idi k_uc,
        std::vector<Candidate> &set_L,
        const idi local_queue_start,
        const idi local_queue_size,
        std::vector<idi> &top_m_candidates,
        //        const idi top_m_candidates_start,
        idi &top_m_candidates_size,
        idi &last_k) const
    {
        idi tmp_last_k = local_queue_size;
        idi tmc_size = 0;
        idi c_i_start = local_queue_start + k_uc;
        idi c_i_bound = local_queue_start + local_queue_size;
        // Pick top-M
        for (idi c_i = c_i_start; c_i < c_i_bound && tmc_size < M; ++c_i)
        {
            if (set_L[c_i].is_checked_)
            {
                continue;
            }
            tmp_last_k = c_i - local_queue_start; // Record the location of the last candidate selected.
            set_L[c_i].is_checked_ = true;
            top_m_candidates[tmc_size++] = set_L[c_i].id_;
        }
        last_k = tmp_last_k;
        top_m_candidates_size = tmc_size;
    }

    /*
     * Function: expand a candidate, visiting its neighbors.
     * Return the lowest adding location.
     */
    idi Searching::expand_one_candidate(
        const int worker_id,
        const idi cand_id,
        const dataf *query_data,
        const distf &dist_bound,
        //        distf &dist_thresh,
        std::vector<Candidate> &set_L,
        const idi local_queue_start,
        idi &local_queue_size,
        const idi &local_queue_capacity,
        boost::dynamic_bitset<> &is_visited,
        uint64_t &local_count_computation)
    //        bool &is_quota_done)
    {
        uint64_t tmp_count_computation = 0;
        //    _mm_prefetch(opt_nsg_graph_ + cand_id * vertex_bytes_ + data_bytes_, _MM_HINT_T0);
        idi *out_edges = (idi *)(opt_nsg_graph_ + cand_id * vertex_bytes_ + data_bytes_);
        idi out_degree = *out_edges++;
        //    if (threads_computations_[q_i] + out_degree >= thread_compuation_quota_) {
        //        is_quota_done = true;
        //        return local_queue_capacity;
        //    }
        //    for (idi n_i = 0; n_i < out_degree; ++n_i) {
        //        _mm_prefetch(opt_nsg_graph_ + out_edges[n_i] * vertex_bytes_, _MM_HINT_T0);
        //    }
        //    tmp_time_pick_top_m += WallTimer::get_time_mark();
        //    uint64_t tmp_last_count_computation = tmp_count_computation;
        idi nk = local_queue_capacity;

        for (idi e_i = 0; e_i < out_degree; ++e_i)
        {
            idi nb_id = out_edges[e_i];
            { // Sequential edition
                if (is_visited[nb_id])
                {
                    continue;
                }
                is_visited[nb_id] = true;
            }

            auto *nb_data = reinterpret_cast<dataf *>(opt_nsg_graph_ + nb_id * vertex_bytes_);
            dataf norm = *nb_data++;
            ++tmp_count_computation;
            distf dist = compute_distance_with_norm(nb_data, query_data, norm);

            if (dist > dist_bound)
            {
                //        if (dist > dist_bound || dist > dist_thresh) {
                continue;
            }
            Candidate cand(nb_id, dist, false);
            // Add to the local queue.
            idi r = add_into_queue(
                set_L,
                local_queue_start,
                local_queue_size,
                local_queue_capacity,
                cand);
            if (r < nk)
            {
                nk = r;
            }
            //        if (num_threads_ - 1 != worker_id
            //            && local_queue_size > index_thresh_
            //            && set_L[local_queue_start + index_thresh_].distance_ < dist_thresh) {
            //            dist_thresh = set_L[local_queue_start + index_thresh_].distance_;
            //        }
        }
        //    threads_computations_[q_i] += tmp_count_computation - tmp_last_count_computation;
        //    if (threads_computations_[q_i] >= thread_compuation_quota_) {
        //        is_quota_done = true;
        //    }
        local_count_computation += tmp_count_computation;

        return nk;
    }

    void Searching::initialize_set_L_para(
        const dataf *query_data,
        const idi L,
        std::vector<Candidate> &set_L,
        const idi set_L_start,
        idi &set_L_size,
        const std::vector<idi> &init_ids,
        boost::dynamic_bitset<> &is_visited)
    {
        // #pragma omp parallel for
        for (idi c_i = 0; c_i < L; ++c_i)
        {
            is_visited[init_ids[c_i]] = true;
        }

        // #pragma omp parallel for
        //         for (idi v_i = 0; v_i < L; ++v_i) {
        //             idi v_id = init_ids[v_i];
        //             _mm_prefetch(opt_nsg_graph_ + v_id * vertex_bytes_, _MM_HINT_T0);
        //         }

        // Get the distances of all candidates, store in the set set_L.
        uint64_t tmp_count_computation = 0;
#pragma omp parallel for reduction(+ : tmp_count_computation)
        for (unsigned i = 0; i < L; i++)
        {
            unsigned v_id = init_ids[i];
            auto *v_data = reinterpret_cast<dataf *>(opt_nsg_graph_ + v_id * vertex_bytes_);
            dataf norm = *v_data++;
            ++tmp_count_computation;
            distf dist = compute_distance_with_norm(v_data, query_data, norm);
            set_L[set_L_start + i] = Candidate(v_id, dist, false); // False means not checked.
        }
        count_distance_computation_ += tmp_count_computation;
        //        threads_computations_[0] += tmp_count_computation;
        //        tmp_count_computation = 0;
        std::sort(
            set_L.begin() + set_L_start,
            set_L.begin() + set_L_start + L);
        set_L_size = L;
    }

    void Searching::para_search_PSS_v5_dist_thresh_profiling(
        //        const idi M,
        //        const idi worker_M,
        const idi query_id,
        const idi K,
        const idi L,
        std::vector<Candidate> &set_L,
        const std::vector<idi> &init_ids,
        std::vector<idi> &set_K,
        const idi local_queue_capacity, // Maximum size of local queue
        const std::vector<idi> &local_queues_starts,
        std::vector<idi> &local_queues_sizes, // Sizes of local queue
        boost::dynamic_bitset<> &is_visited,
        const idi subsearch_iterations)
    {
#ifdef BREAKDOWN_PRINT
        time_seq_ -= WallTimer::get_time_mark();
#endif
        const idi master_queue_start = local_queues_starts[num_threads_ - 1];
        idi &master_queue_size = local_queues_sizes[num_threads_ - 1];
        const dataf *query_data = queries_load_ + query_id * dimension_;
        uint64_t local_dist_comps[num_threads_];
        for (int i = 0; i < num_threads_; i++)
            local_dist_comps[i] = 0;
        // Initialization Phase
        initialize_set_L_para(
            query_data,
            L,
            //            local_queue_capacity,
            set_L,
            master_queue_start,
            master_queue_size,
            init_ids,
            is_visited);

        const distf &last_dist = set_L[master_queue_start + master_queue_size - 1].distance_;
        idi iter = 0; // for debug
#ifdef BREAKDOWN_PRINT
        time_seq_ += WallTimer::get_time_mark();
#endif

        {
#ifdef BREAKDOWN_PRINT
            time_seq_ -= WallTimer::get_time_mark();
#endif
            idi k_master = 0; // Index of first unchecked candidate.
            idi para_iter = 0;
            uint64_t tmp_count_computation = 0;
            //        uint8_t count_workers_done = 0;

            // Sequential Start
            bool no_need_to_continue = false;
            {
                //            distf &last_dist = set_L[master_queue_start + master_queue_size - 1].distance_;
                idi r;
                //            idi seq_iter_bound = num_threads_;
                const idi seq_iter_bound = 1;
                while (iter < seq_iter_bound)
                {
                    ++iter;
                    if (k_master == L)
                    {
                        no_need_to_continue = true;
                        break;
                    }
                    //                distf dist_thresh = last_dist;
                    auto &cand = set_L[master_queue_start + k_master];
                    if (!cand.is_checked_)
                    {
                        cand.is_checked_ = true;
                        idi cand_id = cand.id_;
                        r = expand_one_candidate(
                            0,
                            //                            num_threads_ - 1,
                            cand_id,
                            query_data,
                            last_dist,
                            //                            dist_thresh,
                            set_L,
                            master_queue_start,
                            master_queue_size,
                            L,
                            is_visited,
                            tmp_count_computation);
                        //                        is_quota_done);
                        count_distance_computation_ += tmp_count_computation;
                        max_distance_computation_ += tmp_count_computation;
                        tmp_count_computation = 0;
                        ++count_hops_;
                    }
                    else
                    {
                        r = L;
                    }
                    if (r <= k_master)
                    {
                        k_master = r;
                    }
                    else
                    {
                        ++k_master;
                    }
                }
            }
//        idi index_th = L - 1;
//        idi index_th = K / 3;
//        idi index_th = K * 2 / 3;
//        idi index_th= K;
//        idi index_th= K * 5 / 3 / num_threads_;
//        if (index_th >= local_queue_capacity) {
//            index_th = local_queue_capacity - 1;
//        }
//        if (index_th >= L) {
//            index_th = L - 1;
//        }
#ifdef BREAKDOWN_PRINT
            time_seq_ += WallTimer::get_time_mark();
#endif
            // Parallel Phase
            while (!no_need_to_continue) // no_need_to_continue
            {
#ifdef BREAKDOWN_PRINT
                time_merge_ -= WallTimer::get_time_mark();
#endif
                ++iter;
                ++para_iter;
                //            {//test
                //                printf("------- iter: %u -------\n", iter);
                //            }
                // Pick and copy top-M unchecked from Master to other workers
                if (!pick_top_m_to_workers(
                        //                    M,
                        set_L,
                        local_queues_starts,
                        local_queues_sizes,
                        local_queue_capacity,
                        k_master))
                {
#ifdef BREAKDOWN_PRINT
                    time_merge_ += WallTimer::get_time_mark();
#endif
                    break;
                }
//            distf dist_thresh = last_dist;
//            distf dist_thresh = set_L[master_queue_start + master_queue_size - 1].distance_;
#ifdef BREAKDOWN_PRINT
                time_merge_ += WallTimer::get_time_mark();
#endif

//            count_workers_done = 0;
// Expand
#ifdef BREAKDOWN_PRINT
                time_expand_ -= WallTimer::get_time_mark();
#endif

#pragma omp parallel reduction(+ : tmp_count_computation, count_hops_) //
                {
                    //                bool is_quota_done = false;
                    int w_i = omp_get_thread_num();
                    const idi local_queue_start = local_queues_starts[w_i];
                    idi &local_queue_size = local_queues_sizes[w_i];
                    const idi queue_capacity = num_threads_ - 1 != w_i ? local_queue_capacity : L;
                    idi k_uc = num_threads_ - 1 != w_i ? 0 : k_master;
                    idi cand_id;
                    idi r;
                    idi worker_iter = 0;
                    while (worker_iter < subsearch_iterations && k_uc < local_queue_size)
                    {
                        auto &cand = set_L[local_queue_start + k_uc];
                        if (!cand.is_checked_)
                        {
                            cand.is_checked_ = true;
                            ++worker_iter;
                            ++count_hops_;
                            cand_id = cand.id_;
                            r = expand_one_candidate(
                                w_i,
                                cand_id,
                                query_data,
                                last_dist,
                                //                                dist_thresh,
                                set_L,
                                local_queue_start,
                                local_queue_size,
                                queue_capacity,
                                is_visited,
                                local_dist_comps[w_i]); // tmp_count_computation
                            if (r <= k_uc)
                            {
                                k_uc = r;
                            }
                            else
                            {
                                ++k_uc;
                            }
                            //                        if (
                            ////                        if (w_i != num_threads_ - 1 &&
                            //                                local_queue_size > index_thresh_
                            //                                && set_L[local_queue_start + index_thresh_].distance_ < dist_thresh) {
                            //                            dist_thresh = set_L[local_queue_start + index_thresh_].distance_;
                            //                        }
                            ////                        if (r >= index_thresh_) {
                            ////                            break;
                            ////                        }
                        }
                        else
                        {
                            ++k_uc;
                        }

                    } // Expand Top-1
                    if (num_threads_ - 1 == w_i)
                    {
                        k_master = k_uc;
                    }
                    //                if (k_uc == local_queue_size || is_quota_done) {
                    //                    ++count_workers_done;
                    //                }
                } // Workers
                // count_distance_computation_ += tmp_count_computation;
                // tmp_count_computation = 0;
#ifdef BREAKDOWN_PRINT
                time_expand_ += WallTimer::get_time_mark();
                time_merge_ -= WallTimer::get_time_mark();
#endif
                // Merge
                {
                    ++count_merge_;
                    idi r = merge_all_queues_to_master(
                        set_L,
                        local_queues_starts,
                        local_queues_sizes,
                        local_queue_capacity,
                        L);
                    if (r <= k_master)
                    {
                        k_master = r;
                    }
                }
#ifdef BREAKDOWN_PRINT
                time_merge_ += WallTimer::get_time_mark();
#endif
            } // Search Iterations
        } // Parallel Phase

        //    count_iterations_ += iter;
        float mincomps = 1000000, maxcomps = 0;
        for (int i = 0; i < num_threads_; i++)
        {
            count_distance_computation_ += local_dist_comps[i];
            if (local_dist_comps[i] < mincomps)
                mincomps = local_dist_comps[i];
            if (local_dist_comps[i] > maxcomps)
                maxcomps = local_dist_comps[i];
        }
        max_distance_computation_ += maxcomps;
        ub_ratio += maxcomps / mincomps;
#ifdef BREAKDOWN_PRINT
        time_seq_ -= WallTimer::get_time_mark();
#endif
#pragma omp parallel for
        for (idi k_i = 0; k_i < K; ++k_i)
        {
            set_K[k_i] = set_L[k_i + master_queue_start].id_;
        }
        { // Reset
            is_visited.reset();
            //        is_visited.clear_all();
            //        std::fill(local_queues_sizes.begin(), local_queues_sizes.end(), 0);
            //        std::fill(threads_computations_.begin(), threads_computations_.end(), 0);
        }
//    {//test
//        printf("query_id: %u "
//               "iter: %u\n",
//               query_id,
//               iter);
//    }
//    {//test
//        for (idi k_i = 0; k_i < K; ++k_i) {
//            printf("%u\n", set_K[k_i]);
//        }
//        if (1000 == query_id) {
//            exit(1);
//        }
//    }
#ifdef BREAKDOWN_PRINT
        time_seq_ += WallTimer::get_time_mark();
#endif
    }

    void Searching::para_search_PSS_v5_dist_thresh_profiling_nosync(
        //        const idi M,
        //        const idi worker_M,
        const idi query_id,
        const idi K,
        const idi L,
        std::vector<Candidate> &set_L,
        const std::vector<idi> &init_ids,
        std::vector<idi> &set_K,
        const idi local_queue_capacity, // Maximum size of local queue
        const std::vector<idi> &local_queues_starts,
        std::vector<idi> &local_queues_sizes, // Sizes of local queue
        boost::dynamic_bitset<> &is_visited,
        const idi subsearch_iterations)
    {
#ifdef BREAKDOWN_PRINT
        time_seq_ -= WallTimer::get_time_mark();
#endif
        const idi master_queue_start = local_queues_starts[num_threads_ - 1];
        idi &master_queue_size = local_queues_sizes[num_threads_ - 1];
        const dataf *query_data = queries_load_ + query_id * dimension_;
        uint64_t local_dist_comps[num_threads_];
        for (int i = 0; i < num_threads_; i++)
            local_dist_comps[i] = 0;
        // Initialization Phase
        initialize_set_L_para(
            query_data,
            L,
            //            local_queue_capacity,
            set_L,
            master_queue_start,
            master_queue_size,
            init_ids,
            is_visited);

        const distf &last_dist = set_L[master_queue_start + master_queue_size - 1].distance_;
        idi iter = 0; // for debug
#ifdef BREAKDOWN_PRINT
        time_seq_ += WallTimer::get_time_mark();
#endif

        {
#ifdef BREAKDOWN_PRINT
            time_seq_ -= WallTimer::get_time_mark();
#endif
            idi k_master = 0; // Index of first unchecked candidate.
            idi para_iter = 0;
            uint64_t tmp_count_computation = 0;
            //        uint8_t count_workers_done = 0;

            // Sequential Start
            bool no_need_to_continue = false;
            {
                //            distf &last_dist = set_L[master_queue_start + master_queue_size - 1].distance_;
                idi r;
                //            idi seq_iter_bound = num_threads_;
                const idi seq_iter_bound = 1;
                while (iter < seq_iter_bound)
                {
                    ++iter;
                    if (k_master == L)
                    {
                        no_need_to_continue = true;
                        break;
                    }
                    //                distf dist_thresh = last_dist;
                    auto &cand = set_L[master_queue_start + k_master];
                    if (!cand.is_checked_)
                    {
                        cand.is_checked_ = true;
                        idi cand_id = cand.id_;
                        r = expand_one_candidate(
                            0,
                            //                            num_threads_ - 1,
                            cand_id,
                            query_data,
                            last_dist,
                            //                            dist_thresh,
                            set_L,
                            master_queue_start,
                            master_queue_size,
                            L,
                            is_visited,
                            tmp_count_computation);
                        //                        is_quota_done);
                        count_distance_computation_ += tmp_count_computation;
                        max_distance_computation_ += tmp_count_computation;
                        tmp_count_computation = 0;
                        ++count_hops_;
                    }
                    else
                    {
                        r = L;
                    }
                    if (r <= k_master)
                    {
                        k_master = r;
                    }
                    else
                    {
                        ++k_master;
                    }
                }
            }

#ifdef BREAKDOWN_PRINT
            time_seq_ += WallTimer::get_time_mark();
#endif
            // Parallel Phase
            while (!no_need_to_continue) // no_need_to_continue
            {
#ifdef BREAKDOWN_PRINT
                time_merge_ -= WallTimer::get_time_mark();
#endif
                ++iter;
                ++para_iter;
                //            {//test
                //                printf("------- iter: %u -------\n", iter);
                //            }
                // Pick and copy top-M unchecked from Master to other workers
                if (!pick_top_m_to_workers(
                        //                    M,
                        set_L,
                        local_queues_starts,
                        local_queues_sizes,
                        local_queue_capacity,
                        k_master))
                {
#ifdef BREAKDOWN_PRINT
                    time_merge_ += WallTimer::get_time_mark();
#endif
                    break;
                }
//            distf dist_thresh = last_dist;
//            distf dist_thresh = set_L[master_queue_start + master_queue_size - 1].distance_;
#ifdef BREAKDOWN_PRINT
                time_merge_ += WallTimer::get_time_mark();
#endif

//            count_workers_done = 0;
// Expand
#ifdef BREAKDOWN_PRINT
                time_expand_ -= WallTimer::get_time_mark();
#endif

#pragma omp parallel reduction(+ : tmp_count_computation, count_hops_) //
                {
                    //                bool is_quota_done = false;
                    int w_i = omp_get_thread_num();
                    const idi local_queue_start = local_queues_starts[w_i];
                    idi &local_queue_size = local_queues_sizes[w_i];
                    const idi queue_capacity = num_threads_ - 1 != w_i ? local_queue_capacity : L;
                    idi k_uc = num_threads_ - 1 != w_i ? 0 : k_master;
                    idi cand_id;
                    idi r;
                    idi worker_iter = 0;
                    while (k_uc < local_queue_size)
                    {
                        auto &cand = set_L[local_queue_start + k_uc];
                        if (!cand.is_checked_)
                        {
                            cand.is_checked_ = true;
                            ++worker_iter;
                            ++count_hops_;
                            cand_id = cand.id_;
                            r = expand_one_candidate(
                                w_i,
                                cand_id,
                                query_data,
                                last_dist,
                                //                                dist_thresh,
                                set_L,
                                local_queue_start,
                                local_queue_size,
                                queue_capacity,
                                is_visited,
                                local_dist_comps[w_i]); // tmp_count_computation
                            if (r <= k_uc)
                            {
                                k_uc = r;
                            }
                            else
                            {
                                ++k_uc;
                            }
                        }
                        else
                        {
                            ++k_uc;
                        }

                    } // Expand Top-1
                    if (num_threads_ - 1 == w_i)
                    {
                        k_master = k_uc;
                    }
                } // Workers
                // count_distance_computation_ += tmp_count_computation;
                // tmp_count_computation = 0;
#ifdef BREAKDOWN_PRINT
                time_expand_ += WallTimer::get_time_mark();
                time_merge_ -= WallTimer::get_time_mark();
#endif
                // Merge
                {
                    no_need_to_continue = true;
                    ++count_merge_;
                    idi r = merge_all_queues_to_master(
                        set_L,
                        local_queues_starts,
                        local_queues_sizes,
                        local_queue_capacity,
                        L);
                    if (r <= k_master)
                    {
                        k_master = r;
                    }
                }
#ifdef BREAKDOWN_PRINT
                time_merge_ += WallTimer::get_time_mark();
#endif
            } // Search Iterations
        } // Parallel Phase

        //    count_iterations_ += iter;
        float mincomps = 1000000, maxcomps = 0;
        for (int i = 0; i < num_threads_; i++)
        {
            count_distance_computation_ += local_dist_comps[i];
            if (local_dist_comps[i] < mincomps)
                mincomps = local_dist_comps[i];
            if (local_dist_comps[i] > maxcomps)
                maxcomps = local_dist_comps[i];
        }
        max_distance_computation_ += maxcomps;
        ub_ratio += maxcomps / mincomps;
#ifdef BREAKDOWN_PRINT
        time_seq_ -= WallTimer::get_time_mark();
#endif
#pragma omp parallel for
        for (idi k_i = 0; k_i < K; ++k_i)
        {
            set_K[k_i] = set_L[k_i + master_queue_start].id_;
        }

        { // Reset
            is_visited.reset();
        }

#ifdef BREAKDOWN_PRINT
        time_seq_ += WallTimer::get_time_mark();
#endif
    }
} // namespace PANNS