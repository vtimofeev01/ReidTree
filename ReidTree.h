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

        pTNode root_;
        Id counter = 0;
        // to check added Node length and to reserve memory
        size_t default_vec_len = 0;

        size_t max_node_size = 3;

        ReidTree() = default;

        // get_object VecParameter return similarity
        void add_vector(std::vector<T> &data_) {
            auto insert_node = std::make_shared<TNode>(++counter, data_);
            if (root_ == nullptr) { default_vec_len = data_.size();
                root_ = std::make_shared<TNode>(++counter, std::vector<T>{});}
            else
                assert(data_.size() == default_vec_len);

            pTNode pointer = root_;


            while (pointer != nullptr) { // !cur_node_list->empty()
                // if node is not full
                if (pointer->children.size() < max_node_size) {
                    pointer->children[insert_node.n_id] = insert_node;
                    pointer = nullptr;
                    break;
                }
                // fill initial cross_cost
                if (pointer->cross_cost.empty()) {
                    pointer->children[insert_node.n_id] = insert_node;
                    for (auto &[id1, node1]: pointer->children)
                        for (auto &[id2, node2]: pointer->children) {
                            if (id1 >= id2) continue;
                            pointer->cross_cost[min(id1, id2), max(id1, id2)] = vec_to_vec_similarity(node1->n_data,
                                                                                                      node2->n_data);
                        }
                } else {
                    for (auto &[id1, node1]: pointer->children) {
                        pointer->cross_cost[min(id1, insert_node->n_id), max(id1,
                                                                             insert_node->n_id)] = vec_to_vec_similarity(
                                node1->n_data, insert_node->n_data);
                    }
                    pointer->children[insert_node.n_id] = insert_node;
                }
                // find worth pair to eliminate
                std::map<int, float> cross_sum;
                std::pair<int, int> max_ixs;
                float max_val{-10.};
                for (auto [ids, cs]: pointer->cross_cost) {
                    if (cs > max_val) {
                        max_val = cs;
                        max_ixs = ids;
                    }

                    cross_sum[ids.first] += cs;
                    cross_sum[ids.second] += cs;
                }
                // push up selected
                int ix_up, ix_stay;
                if (cross_sum[max_ixs.first] > cross_sum[max_ixs.second]) {
                    ix_up = max_ixs.first;
                    ix_stay = max_ixs.second;
                } else {
                    ix_up = max_ixs.second;
                    ix_stay = max_ixs.first;
                }

                // set pointer to stay and new det to up
                insert_node = pointer->children[ix_up];
                pointer->children.erase(ix_up);
                pointer = pointer->children[ix_stay];

            }

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

        T operator&(ReidTree &b) {
            reid_tree::FindMaxQueue<pTNode2, T> q(2 * (1 - similarity_for_same));
            T cs;
            int node_calculated{1};

            // fill with roots
            if (!root_->children.empty() && !b.root_->children.empty()) {
                for (auto &[ix1, cd1]: root_->children)
                    for (auto &[ix2, cd2]: b.root_->children) {
                        cs = reid_tree::vec_to_vec_similarity(cd1.n_data, cd2.n_data);
                        if (cs > similarity_for_same) return 1;
                        q.Push(pTNode2(cd1, cd2), cs);
                    }
            }

            while (!q.empty()) {
                pTNode2 qO = q.get_object();
                pTNode cn1 = qO.first;
                pTNode cn2 = qO.second;
                // add all children
                if (!cn1->children.empty() && !cn2->children.empty()) {
                    for (auto &cd1: cn1->children)
                        for (auto &cd2: cn2->children) {
                            cs = reid_tree::vec_to_vec_similarity(cd1.n_data, cd2.n_data, cd1.n_data.size());
                            if (cs > similarity_for_same) return 1;
                            q.Push(pTNode2(&cd1, &cd2), cs);
                        }
                }
                // add parent to child
                if (!cn1->children.empty())
                    for (auto &cd1: cn1->children) {
                        cs = reid_tree::vec_to_vec_similarity(cd1.n_data, cn2->n_data, cd1.n_data.size());
                        if (cs > similarity_for_same) return 1;
                        q.Push(pTNode2(cn2, &cd1), cs);
                    }

                if (!cn2->children.empty())
                    for (auto &cd2: cn2->children) {
                        cs = reid_tree::vec_to_vec_similarity(cn1->n_data, cd2.n_data, cd2.n_data.size());
                        if (cs > similarity_for_same) return 1;
                        q.Push(pTNode2(cn1, &cd2), cs);
                    }
            }

            return q.max_value;
        }

        ~ReidTree() { root_->children.clear(); };

        // outputs to screen tree code in dot format for later look
        [[maybe_unused]] void output_DOT() const {

            std::map<int, std::set<unsigned long>> levels;

            std::cout << "graph tree {" << std::endl;
            std::cout.precision(3);
            std::queue<pTNode> q;

            q.push(root_);

            while (!q.empty()) {
                auto v = q.front();
                auto lvl = (int) (1000 * v->max_node_2_node_difference);
                if (levels.find(lvl) == levels.end()) levels[lvl] = {};
                if (!v->children.empty()) {
                    for (auto &vc: v->children) {
                        auto cs = vec_to_vec_similarity(v->n_data, vc.n_data);
                        std::cout << "    " << v->n_id << " -- " << vc.n_id << " [label=\"" << cs << "\\ll:" << v->level
                                  << "\"]" << std::endl;
                        levels[lvl].insert(vc.n_id);
                        q.push(&vc);
                    }
                }
                q.pop();
            }

            // { rank=same; A1 A2 A3 }
            std::vector<int> level_labels;
            level_labels.reserve(levels.size());
            for (auto &[l, data]: levels) level_labels.push_back(l);


            std::sort(level_labels.begin(), level_labels.end(), [&](auto a, auto b) { return a < b; });
            std::cout << "    ";
            for (auto i = 0; i < level_labels.size(); i++) {
                std::cout << "_" << level_labels[i];
                if (i < level_labels.size() - 1) std::cout << " -- ";
            }
            std::cout << std::endl;

            for (auto &[l, data]: levels) {
                level_labels.push_back(l);
                std::cout << "    { rank=same; _" << l;
                for (auto v: data) std::cout << " " << v;
                std::cout << " }" << std::endl;
            }

//            printf("}");

        };

        bool empty() { return root_ == nullptr; }

        int size() { return counter; }

        void clear() {
            root_->children.clear();
            counter = 1;
        }
    };


}
