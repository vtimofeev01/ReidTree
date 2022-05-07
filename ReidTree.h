#pragma once

#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <queue>
#include "MainDefinitions.h"
#include "Node.h"
#include "FindMaxQueue.h"
#include <cassert>
#include <iostream>
#define pprintf //printf
#define TRACE
namespace reid_tree {

    template<class T>
    T vec_to_vec_similarity(std::vector<T> &vec_a, std::vector<T> &vec_b) {
        T xx = 0, xy = 0, yy = 0;
        auto size1 = vec_a.size();
        for (int index = 0; index < size1; index++) {
            xx += std::pow(vec_a[index], 2.f);
            xy += vec_a[index] * vec_b[index];
            yy += std::pow(vec_b[index], 2.f);
        }
        return (T) xy / ((T) std::sqrt(xx * yy) + 1e-6f);
    }

    template<class T>

    class ReidTree {
        using TNode = Node<T>;
        using pTNode = std::shared_ptr<TNode>;
        using pTNode2 = std::pair<pTNode, pTNode>;
    public:
        T similarity_for_same = .94;
        // when similarity reaches this value vector can not be stored
        T not_to_add = .97;
        // threshold for queue cleaning
//        T q_clear_thr = .1;

        pTNode root_ = nullptr;
        Id counter = 0;
        // to check added Node length and to reserve memory
        size_t default_vec_len = 0;

        size_t max_node_size = 3;
        std::map<std::pair<int, int>, float> cross_cost;

        ReidTree() = default;

        // get_object VecParameter return similarity
        int add_vector(std::vector<T> &data_, bool use_max_pass = true) {

            if (root_ == nullptr) {
                default_vec_len = data_.size();
                root_ = std::make_shared<TNode>(++counter, std::vector<T>{});
            } else
                assert(data_.size() == default_vec_len);
            auto insert_node = std::make_shared<TNode>(++counter, data_);
            pTNode pointer = root_;
            pprintf("new pointer<%i>[%lu] root is on <%i> with child size %lu\n", insert_node->n_id, insert_node->n_data.size(), pointer->n_id, pointer->children.size());
            while (pointer != nullptr) { // !cur_node_list->empty()
                pprintf("   pointer<%i>[%lu] puts to <%i> with child size %lu\n", insert_node->n_id, insert_node->n_data.size(), pointer->n_id, pointer->children.size());
                // if node is not full
                if (pointer->children.size() < max_node_size) {
                    pointer->children[insert_node->n_id] = insert_node;
//                    pointer = nullptr;
                    pprintf("   added Node[%i] to Node[%i] as non_complete with new size %lu\n", insert_node->n_id, pointer->n_id, pointer->children.size());
                    return 1;
                }
                // fill initial cross_cost
                pprintf("  Cross in pointers [");
                for (auto [a, b]: pointer->children) pprintf(" %i", a);
                pprintf(" ] cs is <%i>:", pointer->n_id);

                std::map<int, float> cross_sum;
                std::pair<int, int> max_ixs;
                float max_val{-10.};
                float cs;
                pointer->children[insert_node->n_id] = insert_node;
                for (auto [id1, node1]: pointer->children)
                    for (auto [id2, node2]: pointer->children) {
                        if (id1 >= id2) continue;
                        auto pIx = std::make_pair(std::min(id1, id2), std::max(id1, id2));
                        if (cross_cost[pIx] == 0) cross_cost[pIx] = vec_to_vec_similarity(node1->n_data, node2->n_data);
                        cs = cross_cost[pIx];
                        pprintf(" %i/%i: %f", pIx.first, pIx.second, cross_cost[pIx]);
                        if (cs > max_val) {
                            max_val = cs;
                            max_ixs = pIx;
                        }
                        cross_sum[pIx.first] += cs;
                        cross_sum[pIx.second] += cs;
                    }

                pprintf(" max: %i/%i:%f\n", max_ixs.first, max_ixs.second, max_val);

                // push up selected
                int ix_up, ix_stay;
                if (cross_sum[max_ixs.first] > cross_sum[max_ixs.second]) {
                    ix_up = max_ixs.first;
                    ix_stay = max_ixs.second;
                } else {
                    ix_up = max_ixs.second;
                    ix_stay = max_ixs.first;
                }
                if ((max_val > not_to_add) && use_max_pass) {
                    for (auto [ix, nd]: pointer->children[ix_up]->children) pointer->children[ix_stay]->children[ix] = nd;
                    pointer->children[ix_up]->children.clear();
                    pointer->children.erase(ix_up);
                    return 0;
                }
                else {
                    insert_node = pointer->children[ix_up];
                    pprintf("   <%i(%i)>[%lu] pushed into <%i>\n", insert_node->n_id, ix_up, insert_node->n_data.size(), ix_stay);
                    pointer->children.erase(ix_up);
                    pointer = pointer->children[ix_stay];
                    for (auto [ix, nd]: insert_node->children) pointer->children[ix] = nd;
                    insert_node->children.clear();}
                // set pointer to stay and new det to up



            }
            return 1;
        }


        // find nears value by vec/ comments - output data to insert in .DOT to see what was looked
        [[maybe_unused]] T nearst(std::vector<T> data_) const {
            int nodes_passed = 1;
            reid_tree::FindMaxQueue<pTNode, T> q((1 - similarity_for_same) * 2);
            for (auto &[ix, nd]: root_) q.Push(nd, vec_to_vec_similarity(data_, nd->n_data));
#ifdef RT_OUTPUT_DOT_DATA
            std::vector<unsigned long> best_path;
            std::vector<pTNode> all_path;
            best_path.push_back(root_->n_id);
            all_path.push_back(root_);
#endif
            T cs;
            int res;
            while (!q.empty()) {
                auto *v = q.get_object();
                if (!v->children.empty()) {
                    for (auto &[_, c]: v->children) {
                        cs = vec_to_vec_similarity(data_, c.n_data);
                        res = q.Push(c, cs);
#ifdef RT_OUTPUT_DOT_DATA
                        all_path.push_back(&c);
                        if (res == 2) best_path.push_back(c.n_id);
#endif
                        if (cs > similarity_for_same) return cs;
                        nodes_passed++;
                    }
                }
            }
#ifdef RT_OUTPUT_DOT_DATA
            for (auto v: best_path) std::cout << "  " << v;
            std::cout << std::endl;
            for (auto v: all_path) {
                std::cout << "  " << v->n_id << "[label=\"" << v->n_id << "\\l" << vec_to_node_similarity(data_, *v)
                          << "\" color=";
                if (std::count(best_path.begin(), best_path.end(), v->n_id) == 0) std::cout << "blue";
                else std::cout << "red";
                std::cout << "]" << std::endl;

            }
#endif

            return q.max_value;

        };

        T compare_to_tree(std::shared_ptr<ReidTree> b) {
            reid_tree::FindMaxQueue<pTNode2, T> q(2 * (1 - similarity_for_same));
            T cs;
            int node_calculated{1};

            // fill with roots
            if (root_->children.empty() || b->root_->children.empty()) return (T) -1;

            for (auto &[ix1, cd1]: root_->children)
                for (auto &[ix2, cd2]: b->root_->children) {
                    cs = reid_tree::vec_to_vec_similarity(cd1->n_data, cd2->n_data);
                    node_calculated++;
                    if (cs > similarity_for_same) return 1;
                    q.Push(pTNode2(cd1, cd2), cs);
                }


            while (!q.empty()) {
                auto [node1, node2] = q.get_object();
                // add all children
                if (!node1->children.empty() && !node2->children.empty()) {
                    for (auto [ix1, cd1]: node1->children)
                        for (auto [ix2, cd2]: node2->children) {
                            node_calculated++;
                            cs = reid_tree::vec_to_vec_similarity(cd1->n_data, cd2->n_data);
                            if (cs > similarity_for_same) {
                                printf("nodes:%i\n", node_calculated);
                                return 1;
                            }
                            q.Push(pTNode2(cd1, cd2), cs);
                        }
                }
                // add parent to child
                if (!node1->children.empty())
                    for (auto [ix1, child1]: node1->children) {
                        cs = reid_tree::vec_to_vec_similarity(child1->n_data, node2->n_data);
                        node_calculated++;
                        if (cs > similarity_for_same) {
                            printf("nodes:%i\n", node_calculated);
                            return 1;
                        }
                        q.Push(pTNode2(node2, child1), cs);
                    }

                if (!node2->children.empty())
                    for (auto [ix2, cd2]: node2->children) {
                        cs = reid_tree::vec_to_vec_similarity(node1->n_data, cd2->n_data);
                        node_calculated++;
                        if (cs > similarity_for_same) {
                            printf("nodes:%i\n", node_calculated);
                            return 1;
                        }
                        q.Push(pTNode2(node1, cd2), cs);
                    }
            }
            printf("nodes:%i\n", node_calculated);
            return q.max_value;
        }

        ~ReidTree() { root_->children.clear(); };

        // outputs to screen tree code in dot format for later look
        [[maybe_unused]] void output_DOT() const {

            std::cout << "graph tree {" << std::endl;
            std::cout.precision(3);
            std::queue<pTNode> q;

            q.push(root_);

            while (!q.empty()) {
                auto v = q.front();
                if (!v->children.empty()) {
                    for (auto [_, vc]: v->children) {
                        auto cs = vec_to_vec_similarity(v->n_data, vc->n_data);
                        std::cout << "    " << v->n_id << " -- " << vc->n_id << " [label=\"" << cs << "\"]"
                                  << std::endl;
                        q.push(vc);
                    }
                }
                q.pop();
            }


        };

        bool empty() { return root_ == nullptr; }

        int size() { return counter; }

        void clear() {
            if (root_ == nullptr) return;
            root_->children.clear();
            root_ = nullptr;
            counter = 1;
        }
    };


}
