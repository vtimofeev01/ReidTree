#pragma once

#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <queue>
#include "Node.h"
#include "FindMaxQueue.h"
#include <cassert>
#include <iostream>
#include <memory>

#define add_vector_log //printf
#define tree_comp_log  // printf
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

    template<class T, class T_key>
    class ReidTree {
        using TNode = Node<T, T_key>;
        using pTNode = std::shared_ptr<TNode>;
        using pTNode2 = std::pair<pTNode, pTNode>;
        using queueElement = std::pair<pTNode2, T>;
    public:
        T similarity_for_same = .94;
        // when similarity reaches this value vector can not be stored
        T not_to_add = .99;
        // threshold for queue cleaning

        pTNode root_ = nullptr;
        int counter = 0;
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
            add_vector_log("new pointer<%i>[%lu] root is on <%i> with child size %lu\n", insert_node->n_id, insert_node->n_data.size(), pointer->n_id, pointer->children.size());
            while (pointer != nullptr) { // !cur_node_list->empty()
                add_vector_log("   pointer<%i>[%lu] puts to <%i> with child size %lu\n", insert_node->n_id, insert_node->n_data.size(), pointer->n_id, pointer->children.size());
                // if node is not full
                if (pointer->children.size() < max_node_size) {
                    pointer->children[insert_node->n_id] = insert_node;
//                    pointer = nullptr;
                    add_vector_log("   added Node[%i] to Node[%i] as non_complete with new size %lu\n", insert_node->n_id, pointer->n_id, pointer->children.size());
                    return 1;
                }
                // fill initial cross_cost
                add_vector_log("  Cross in pointers [");
                for (auto [a, b]: pointer->children) add_vector_log(" %i", a);
                add_vector_log(" ] cs is <%i>:", pointer->n_id);

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
                        add_vector_log(" %i/%i: %f", pIx.first, pIx.second, cross_cost[pIx]);
                        if (cs > max_val) {
                            max_val = cs;
                            max_ixs = pIx;
                        }
                        cross_sum[pIx.first] += cs;
                        cross_sum[pIx.second] += cs;
                    }

                add_vector_log(" max: %i/%i:%f\n", max_ixs.first, max_ixs.second, max_val);

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
                } else {
                    insert_node = pointer->children[ix_up];
                    add_vector_log("   <%i(%i)>[%lu] pushed into <%i>\n", insert_node->n_id, ix_up, insert_node->n_data.size(), ix_stay);
                    pointer->children.erase(ix_up);
                    pointer = pointer->children[ix_stay];
                    for (auto [ix, nd]: insert_node->children) pointer->children[ix] = nd;
                    insert_node->children.clear();
                }
                // set pointer to stay and new det to up



            }
            return 1;
        }

        // usd before comparing
        T cross_cs;

        void pre_compare() {
            std::queue<pTNode> queue;
            queue.push(root_);
            T_key next_level;
            pTNode node;
            while (!queue.empty()) {
                node = queue.front();
                queue.pop();
                if (node->children.empty()) {
                    node->min_lvl = 0;
                    node->max_lvl = 1;
                    continue;
                }
                next_level = node->lvl + 1;
                for (auto [a, b]: node->children) {b->lvl = next_level; queue.push(b); b->parent = node;}
                node->min_lvl = 2;
                node->max_lvl = -2;
                if (node->children.size() == 1){
                    node->min_lvl = 1;
                }
                else {
                    for (auto [ix1, cn1]: node->children)
                        for (auto [ix2, cn2]: node->children) {
                            if (ix1 >= ix2) continue;
                            cross_cs = cross_cost[std::make_pair(ix1, ix2)];
                            if (cross_cs == 0) {
                                cross_cs = vec_to_vec_similarity(cn1->n_data, cn2->n_data);
                                cross_cost[std::make_pair(ix1, ix2)] = cross_cs;
                            }
                            if (cross_cs > node->max_lvl) node->max_lvl = cross_cs;
                            if (cross_cs < node->min_lvl) node->min_lvl = cross_cs;
                        }
                }

                if (node->min_lvl < 0 || node->min_lvl == 2) node->min_lvl = 0; // else node->min_lvl = node->min_lvl * node->min_lvl;
                if (node->max_lvl > 1 || node->max_lvl == -2) node->max_lvl = 1; //else node->max_lvl = node->max_lvl * node->max_lvl;
            }
        };

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

        // got [1 1] similarity: 0.000000   calculated:0
        //  [19 15] max sim:0.402037 sim:0.402037 > min_lvl 0.543715 0.585259 both with children added
        //[19 15] max sim:0.402037 sim:0.402037 > min_lvl 0.543715 0.585259 both with children added
//        [19 7] max sim:0.437864 sim:0.432810 > min_lvl 0.543715 0.981039 both with children added
//        [18 8] max sim:0.448734 sim:0.448734 > min_lvl 0.708984 0.000000

        T to_tree(std::shared_ptr<ReidTree> b) {
            // fill with roots
            if (root_ == nullptr || b->root_ == nullptr || root_->children.empty() || b->root_->children.empty())
                return (T) -2;
            auto q_sort = [](const queueElement &a, const queueElement &b) { return a.second < b.second; };
            std::priority_queue<queueElement, std::vector<queueElement>, decltype(q_sort)> p_queue(q_sort);
            T max_similarity{0};
            int node_calculated{0};
            std::set<std::pair<T_key, T_key>> passed;
            p_queue.push(std::make_pair(std::make_pair(root_, b->root_), 1));
            T sim;
            bool needs_to_add;

            std::set<std::pair<T_key, T_key>> check;


            while (!p_queue.empty()) {
                queueElement v = p_queue.top();
                p_queue.pop();
                auto [node1, node2] = v.first;
                tree_comp_log("\ngot [%i %i] similarity: %f   calculated:%i queue:%lu min-max (1) %f-%f (2) %f-%f\n", node1->n_id, node2->n_id, v.second,
                        node_calculated, p_queue.size(), node1->min_lvl, node1->max_lvl, node2->min_lvl, node2->max_lvl);
                auto parent_limit = node1->min_lvl * node2-> min_lvl;
                if ((!node1->children.empty()) && (!node2->children.empty()) && (node1->lvl == node2->lvl)) {
                    tree_comp_log("  same level children: %lu %lu\n", node1->children.size(), node2->children.size());
                    for (auto [ix1, cn1]: node1->children)
                        for (auto [ix2, cn2]: node2->children) {
                            sim = vec_to_vec_similarity(cn1->n_data, cn2->n_data);
                            ++node_calculated;
                            if (sim > max_similarity) max_similarity = sim;
                            if (sim > similarity_for_same) return 2;
                            tree_comp_log("  [%i %i] max sim:%f sim:%f > min_lvl %f %f", cn1->n_id, cn2->n_id, max_similarity,
                                    sim, node1->min_lvl, node2->min_lvl);
                            needs_to_add = (!node1->children.empty()) && (!cn1->children.empty());
                            if (needs_to_add) tree_comp_log(" both with children %lu %lu", cn1->children.size(), cn2->children.size());
                            needs_to_add &= sim > parent_limit;

                            if (needs_to_add) {
                                tree_comp_log(" added");
                                assert(check.find(std::make_pair(cn1->n_id, cn2->n_id)) == check.end());
                                check.insert(std::make_pair(cn1->n_id, cn2->n_id));
                                p_queue.push(std::make_pair(std::make_pair(cn1, cn2), sim));
                            }
                            tree_comp_log("\n");
                        }
                }

                if ((node1->n_id != 1) && (!node2->children.empty()) && (node1->lvl <= node2->lvl)) {
                    parent_limit = node1->parent->min_lvl * node2-> min_lvl;
                    tree_comp_log("  node1 to node2 children\n");
                    for (auto [ix2, cn2]: node2->children) {
                        sim = vec_to_vec_similarity(node1->n_data, cn2->n_data);
                        ++node_calculated;
                        if (sim > max_similarity) max_similarity = sim;
                        if (sim > similarity_for_same) return 2;
                        tree_comp_log("  [%i %i] max sim:%f sim:%f > min_lvl %f %f", node1->n_id, cn2->n_id, max_similarity,
                                sim, node1->min_lvl, node2->min_lvl);
                        needs_to_add = (!cn2->children.empty()) && (!node1->children.empty());
                        if (needs_to_add) tree_comp_log(" both with children");
                        needs_to_add &= sim > parent_limit;
                        if (needs_to_add) {
                            tree_comp_log(" added");
                            auto key_ = std::make_pair(node1->n_id, cn2->n_id);
                            assert(check.find(key_) == check.end());
                            check.insert(key_);
                            p_queue.push(std::make_pair(std::make_pair(node1, cn2), sim));
                        }
                        tree_comp_log("\n");
                    }
                }

                if (node2->n_id != 1 && (!node1->children.empty() && (node1->lvl >= node2->lvl))) {
                    parent_limit = node2->parent->min_lvl * node1-> min_lvl;
                    tree_comp_log("  node2 to node1 children\n");
                    for (auto [ix1, cn1]: node1->children) {
                        sim = vec_to_vec_similarity(cn1->n_data, node2->n_data);
                        ++node_calculated;
                        if (sim > max_similarity) max_similarity = sim;
                        if (sim > similarity_for_same) return 2;
                        tree_comp_log("  [%i %i] max sim:%f sim:%f > min_lvl %f %f", cn1->n_id, node2->n_id, max_similarity,
                                sim, node1->min_lvl, node2->min_lvl);
                        needs_to_add = (!node2->children.empty()) && (!cn1->children.empty());
                        if (needs_to_add) tree_comp_log(" both with children");
                        needs_to_add &= sim > parent_limit;

                        if (needs_to_add) {
                            tree_comp_log(" added");
                            assert(check.find(std::make_pair(cn1->n_id, node2->n_id)) == check.end());
                            check.insert(std::make_pair(cn1->n_id, node2->n_id));
                            p_queue.push(std::make_pair(std::make_pair(cn1, node2), sim));
                        }
                        tree_comp_log("\n");
                    }
                }



            }

            printf("nodes:%i\n queue:%lu", node_calculated, p_queue.size());
            return max_similarity;
//            return q.max_value;
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
                std::cout << v->n_id << " [ label=\"" << v->n_id << "[" << v->lvl << "]:" << v->min_lvl << "-" << v->max_lvl << "\"]"
                          << std::endl;
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

        void clear() {
            if (root_ == nullptr) return;
            root_->children.clear();
            root_ = nullptr;
            counter = 0;
        }
    };


}
