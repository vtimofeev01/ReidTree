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

namespace reid_tree {

    template<class T>
    class ReidTree {
        using TNode = Node<T>;
        using pTNode = TNode *;
        using pTNode2 = std::pair<pTNode, pTNode>;
    public:
        // root will start their children with this cross
        T start_max_node_2_node_cs_level = .5;
        // level to level  Node::max_node_2_node_difference change
        T step_node_2_node = .05;
        // value when similarity reaches no matter to look later
        T similarity_for_same = .94;
        // when similarity reaches this value vector can not be stored
        T not_to_add = .97;
        // threshold for queue cleaning
        T q_clear_thr = .2;

        TNode *root_ = nullptr;
        Id counter = 1;
        // to check added Node length and to reserve memory
        unsigned long default_vec_len = 0;

        ReidTree() = default;

        static T vec_to_node_similarity(std::vector<T> vec, TNode &b) {
            T xx = 0, xy = 0, yy = 0;
            auto size1 = vec.size();
            for (int index = 0; index < size1; index++) {
                xx += std::pow(vec[index], 2.f);
                xy += vec[index] * b.n_data[index];
                yy += std::pow(b.n_data[index], 2.f);
            }
            return (T) xy / ((T) std::sqrt(xx * yy) + 1e-6f);
        }

        [[maybe_unused]] static T vec_to_vec_similarity(std::vector<T> vec_a, std::vector<T> vec_b) {
            T xx = 0, xy = 0, yy = 0;
            auto size1 = vec_a.size();
            for (int index = 0; index < size1; index++) {
                xx += std::pow(vec_a[index], 2.f);
                xy += vec_a[index] * vec_b[index];
                yy += std::pow(vec_b[index], 2.f);
            }
            return (T) xy / ((T) std::sqrt(xx * yy) + 1e-6f);
        }

        // get_object VecParameter return 0 - if not needed to add or Node::n_id = 1 ... - if is added
        [[maybe_unused]] Id add_vector(std::vector<T> data_) {
            T max_child_level = start_max_node_2_node_cs_level;
            if (root_ == nullptr) {
                root_ = new TNode(counter, data_);
                root_->max_node_2_node_difference = max_child_level;
                default_vec_len = data_.size();
                counter++;
                return counter;
            }
            assert(data_.size() == default_vec_len);
            TNode *cur = root_;
            T cur_sim = vec_to_node_similarity(data_, *cur);
            // too similar
            if (cur_sim >= not_to_add) return 0;
            while (cur != nullptr) {
                // not too similar but no children
                if (cur->children.empty()) {
                    cur->children.emplace_back(counter, data_);
                    cur->children.back().max_node_2_node_difference = max_child_level;
                    counter++;
                    return counter;
                }
                // enumerate vectors
                T sim, max_sim = -1;
                int max_ix = -1;
                for (auto i = 0; i < cur->children.size(); i++) {
                    sim = vec_to_node_similarity(data_, cur->children[i]);
                    // if found not_to_add - no more sense
                    if (sim > not_to_add) return 0;
                    // find max similarity
                    if (sim > max_sim) {
                        max_sim = sim;
                        max_ix = i;
                    }

                }
                if (max_sim < cur->max_node_2_node_difference) {
                    cur->children.emplace_back(counter, data_);
                    cur->children.back().max_node_2_node_difference = max_child_level;
                    counter++;
                    return counter;
                } else {
                    cur = &(*cur).children[max_ix];
                    max_child_level += step_node_2_node;
                }
                if (cur_sim > not_to_add) return 0;
            }
            return 0;
        }

        // get_object VecParameter return similarity
        T add_vector_return_cs(std::vector<T> data_) {
            T max_child_level = start_max_node_2_node_cs_level;
            int level{1};
            T max_cs_overall;
            if (root_ == nullptr) {
                root_ = new TNode(counter, data_);
                root_->max_node_2_node_difference = max_child_level;
                default_vec_len = data_.size();
                counter++;
                return (T) 1;
            }
            assert(data_.size() == default_vec_len);
            TNode *cur = root_;
            // check max sim
            max_cs_overall = vec_to_node_similarity(data_, *cur);
            if (max_cs_overall >= not_to_add) return max_cs_overall;

            while (cur != nullptr) {

                // not too similar but no children
                if (cur->children.empty()) {
                    cur->children.emplace_back(counter, data_);
                    cur->children.back().max_node_2_node_difference = max_child_level;
                    cur->children.back().level = level;
                    counter++;
                    return max_cs_overall;
                }
                // enumerate vectors
                T sim, max_sim = -1;
                int max_ix = -1;
                for (auto i = 0; i < cur->children.size(); i++) {
                    sim = vec_to_node_similarity(data_, cur->children[i]);
                    // find max similarity
                    if (sim > max_cs_overall) max_cs_overall = sim;
                    // if found not_to_add - no more sense
                    if (sim > not_to_add) return max_cs_overall;
                    if (sim > max_sim) {
                        max_sim = sim;
                        max_ix = i;
                    }
                }
                if (max_sim < cur->max_node_2_node_difference) {
                    cur->children.emplace_back(counter, data_);
                    cur->children.back().max_node_2_node_difference = max_child_level;
                    cur->children.back().level = level;
                    counter++;
                    return max_cs_overall;
                } else {
                    cur = &(*cur).children[max_ix];
                    max_child_level += step_node_2_node;
                    level += 1;
                }
                if (max_cs_overall > not_to_add) return max_cs_overall;
            }
            return max_cs_overall;
        }

        // find nears value by vec/ comments - output data to insert in .DOT to see what was looked
        [[maybe_unused]] T nearst(std::vector<T> data_) const {
            int nodes_passed = 1;
            reid_tree::FindMaxQueue<pTNode, T> q(step_node_2_node * 2);
            q.Push(root_, vec_to_node_similarity(data_, *root_));
#ifdef RT_OUTPUT_DOT_DATA
            std::vector<unsigned long> best_path;
            std::vector<pTNode> all_path;
            best_path.push_back(root_->n_id);
            all_path.push_back(root_);
#endif
            while (!q.empty()) {
                auto *v = q.get_object();
                if (!v->children.empty()) {
                    for (auto &c: v->children) {
                        auto cs = vec_to_node_similarity(data_, c);
                        auto res = q.Push(&c, cs);
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
            reid_tree::FindMaxQueue<pTNode2, T> q(q_clear_thr);
            T cs;
            int node_calculated{1};
            cs = vec_to_vec_similarity(root_->n_data, b.root_->n_data);
//            printf("start cs:%f", cs);
            if (cs > similarity_for_same) return 1;
            q.Push(pTNode2(root_, b.root_), cs);
            while (!q.empty()) {
                pTNode2 qO = q.get_object();
                pTNode cn1 = qO.first;
                pTNode cn2 = qO.second;
                // same level
                if (cn1->level == cn2->level) {
                    // add all children
                    if (!cn1->children.empty() && !cn2->children.empty()) {
                        for (auto &cd1: cn1->children)
                            for (auto &cd2: cn2->children) {
                                cs = vec_to_vec_similarity(cd1.n_data, cd2.n_data);
                                if (cs > similarity_for_same) return 1;
                                q.Push(pTNode2(&cd1, &cd2), cs);
                            }
                    }
                    // add parent to child
                    if (!cn1->children.empty())
                        for (auto &cd1: cn1->children) {
                            cs = vec_to_vec_similarity(cd1.n_data, cn2->n_data);
                            if (cs > similarity_for_same) return 1;
                            q.Push(pTNode2(cn2, &cd1), cs);
                        }

                    if (!cn2->children.empty())
                        for (auto &cd2: cn2->children) {
                            cs = vec_to_vec_similarity(cn1->n_data, cd2.n_data);
                            if (cs > similarity_for_same) return 1;
                            q.Push(pTNode2(cn1, &cd2), cs);
                        }
                }
                else {
                    // compare "old" to new child
                    if (!cn2->children.empty())
                        for (auto &cd2: cn2->children) {
                            cs = vec_to_vec_similarity(cn1->n_data, cd2.n_data);
                            if (cs > similarity_for_same) return 1;
                            q.Push(pTNode2(cn1, &cd2), cs);
                        }
                }


            }
            return q.max_value;
        }

        T operator&(std::vector<std::vector<T>> &b) {
            T max_sim{0};
            T cs;
            for (auto &a: b) {
                cs = nearst(a);
                if (cs > max_sim) max_sim = cs;
            }
            return max_sim;
        }

        ~ReidTree() { delete root_; };

        // outputs to screen tree code in dot format for later look
        [[maybe_unused]] void output_DOT() const {

            std::map<int, std::set<unsigned long>> levels;

            std::cout << "graph tree {" << std::endl;
            std::cout.precision(3);
            std::queue<pTNode> q;

            q.push(root_);
            levels[(int) (1000 * (root_->max_node_2_node_difference - step_node_2_node))].insert(root_->n_id);
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
    };

}
