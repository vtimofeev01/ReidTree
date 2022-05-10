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
#define add_btree_vector_log // printf
#define tree_comp_log  // printf
#define btree_comp_log  //printf
namespace reid_tree {

    template<class T>
    T vec_to_vec_similarity(std::vector<T> &vec_a, std::vector<T> &vec_b) {
        T xx = 0, xy = 0, yy = 0;
        auto size1 = vec_a.size();
        for (int index = 0; index < size1; index++) {
            xx += vec_a[index] * vec_a[index];
            xy += vec_a[index] * vec_b[index];
            yy += vec_b[index] * vec_b[index];
        }
        return (T) xy / ((T) std::sqrt(xx * yy) + 1e-6f);
    }

    template<class T, class T_key, class TNode = reid_tree::Node<T, T_key>>
    class ReidTree {
        using pTNode = std::shared_ptr<TNode>;
        using pTNode2 = std::pair<pTNode, pTNode>;
        using queuePairElement = std::pair<pTNode2, T>;
    public:

        T not_to_add = .99;
        // threshold for queue cleaning

        pTNode root_ = nullptr;
        int counter = 0;
        // to check added Node length and to reserve memory
        size_t default_vec_len = 0;

        size_t max_node_size = 2;
        std::map<std::pair<int, int>, float> cross_cost;

        ReidTree() = default;

        // get_object VecParameter return similarity
        int add_idents_to_tree(std::vector<T> &data_, bool use_max_pass = true) {

            if (root_ == nullptr) {
                default_vec_len = data_.size();
                root_ = std::make_shared<TNode>(++counter, std::vector<T>{});
                assert(counter == 1);
            } else
                assert(data_.size() == default_vec_len);
            auto insert_node = std::make_shared<TNode>(++counter, data_);
            pTNode pointer = root_;
            add_vector_log("new pointer<%i>[%lu] root is on <%i> with child size %lu\n", insert_node->id, insert_node->data.size(), pointer->id, pointer->children.size());
            while (pointer != nullptr) { // !cur_node_list->empty()
                add_vector_log("   pointer<%i>[%lu] puts to <%i> with child size %lu\n", insert_node->id, insert_node->data.size(), pointer->id, pointer->children.size());
                // if node is not full
                if (pointer->children.size() < max_node_size) {
                    pointer->children[insert_node->id] = insert_node;
//                    pointer = nullptr;
                    add_vector_log("   added Node[%i] to Node[%i] as non_complete with new size %lu\n", insert_node->id, pointer->id, pointer->children.size());
                    return 1;
                }
                // fill initial cross_cost
                add_vector_log("  Cross in pointers [");
                for (auto [a, b]: pointer->children) add_vector_log(" %i", a);
                add_vector_log(" ] cs is <%i>:", pointer->id);

                std::map<int, float> cross_sum;
                std::pair<int, int> max_ixs;
                float max_val{-10.};
                float cs;
                pointer->children[insert_node->id] = insert_node;
                for (auto [id1, node1]: pointer->children)
                    for (auto [id2, node2]: pointer->children) {
                        if (id1 >= id2) continue;
                        auto pIx = std::make_pair(std::min(id1, id2), std::max(id1, id2));
                        if (cross_cost[pIx] == 0) cross_cost[pIx] = Node2NodeCompare(node1, node2);
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
                    add_vector_log("   <%i(%i)>[%lu] pushed into <%i>\n", insert_node->id, ix_up, insert_node->data.size(), ix_stay);
                    pointer->children.erase(ix_up);
                    pointer = pointer->children[ix_stay];
                    for (auto [ix, nd]: insert_node->children) pointer->children[ix] = nd;
                    insert_node->children.clear();
                }
            }
            return 1;
        }

        // usd before comparing
        T cross_cs;
        // before to run searches must be executed
        void pre_compare() {
            std::queue<pTNode> queue;
            queue.push(root_);
            T_key next_level;
            pTNode node;
            while (!queue.empty()) {
                node = queue.front();
                queue.pop();
                if (node->children.empty()) {
                    node->min_chd_similarity = 0;
                    node->max_chd_similarity = 1;
                    continue;
                }
                next_level = node->level + 1;
                for (auto [a, b]: node->children) {b->level = next_level; queue.push(b); b->parent = node;}
                node->min_chd_similarity = 2;
                node->max_chd_similarity = -2;
                if (node->children.size() == 1){
                    node->min_chd_similarity = 1;
                }
                else {
                    for (auto [ix1, cn1]: node->children)
                        for (auto [ix2, cn2]: node->children) {
                            if (ix1 >= ix2) continue;
                            cross_cs = cross_cost[std::make_pair(ix1, ix2)];
                            if (cross_cs == 0) {
                                cross_cs = Node2NodeCompare(cn1, cn2);
                                cross_cost[std::make_pair(ix1, ix2)] = cross_cs;
                            }
                            if (cross_cs > node->max_chd_similarity) node->max_chd_similarity = cross_cs;
                            if (cross_cs < node->min_chd_similarity) node->min_chd_similarity = cross_cs;
                        }
                }

                if (node->min_chd_similarity < 0 || node->min_chd_similarity == 2) node->min_chd_similarity = 0; // else node->min_lvl = node->min_lvl * node->min_lvl;
                if (node->max_chd_similarity > 1 || node->max_chd_similarity == -2) node->max_chd_similarity = 1; //else node->max_lvl = node->max_lvl * node->max_lvl;
            }
        };

        T Node2NodeCompare(const pTNode&  n1, const pTNode&  n2){
            return vec_to_vec_similarity(n1->data, n2->data);
        }

        // find nears value by vec/ comments - output data to insert in .DOT to see what was looked
         T nearst(std::vector<T> data_, T distance) const {
            T similarity_for_same = .94;
            int nodes_passed = 1;
            reid_tree::FindMaxQueue<pTNode, T> q((1 - similarity_for_same) * 2);
            for (auto &[ix, nd]: root_) q.Push(nd, vec_to_vec_similarity(data_, nd->data));

            T cs;
            int res;
            while (!q.empty()) {
                auto *v = q.get_object();
                if (!v->children.empty()) {
                    for (auto &[_, c]: v->children) {
                        cs = vec_to_vec_similarity(data_, c.data);
                        res = q.Push(c, cs);
                        if (cs > similarity_for_same) return cs;
                        nodes_passed++;
                    }
                }
            }

            return q.max_value;

        };

        T to_tree(std::shared_ptr<ReidTree> b, T similarity_for_same=.95) {
            // fill with roots
            if (root_ == nullptr || b->root_ == nullptr || root_->children.empty() || b->root_->children.empty())
                return (T) -2;
            auto q_sort = [](const queuePairElement &a, const queuePairElement &b) { return a.second < b.second; };
            std::priority_queue<queuePairElement, std::vector<queuePairElement>, decltype(q_sort)> p_queue(q_sort);
            T max_similarity{0};
            int node_calculated{0};
            std::set<std::pair<T_key, T_key>> passed;
            p_queue.push(std::make_pair(std::make_pair(root_, b->root_), 1));
            T sim;
            bool needs_to_add;

            std::set<std::pair<T_key, T_key>> check;


            while (!p_queue.empty()) {
                queuePairElement v = p_queue.top();
                p_queue.pop();
                auto [node1, node2] = v.first;
                tree_comp_log("\ngot [%i %i] similarity: %f   calculated:%i queue:%lu min-max (1) %f-%f (2) %f-%f\n", node1->id, node2->id, v.second,
                        node_calculated, p_queue.size(), node1->min_chd_similarity, node1->max_chd_similarity, node2->min_chd_similarity, node2->max_chd_similarity);
                auto parent_limit = node1->min_chd_similarity * node2-> min_chd_similarity;
                if ((!node1->children.empty()) && (!node2->children.empty()) && (node1->level == node2->level)) {
                    tree_comp_log("  same level children: %lu %lu\n", node1->children.size(), node2->children.size());
                    for (auto [ix1, cn1]: node1->children)
                        for (auto [ix2, cn2]: node2->children) {
                            sim = Node2NodeCompare(cn1, cn2);
                            ++node_calculated;
                            if (sim > max_similarity) max_similarity = sim;
                            if (sim > similarity_for_same) return 2;
                            tree_comp_log("  [%i %i] max sim:%f sim:%f > min_lvl %f %f", cn1->id, cn2->id, max_similarity,
                                    sim, node1->min_chd_similarity, node2->min_chd_similarity);
                            needs_to_add = (!node1->children.empty()) && (!cn1->children.empty());
                            if (needs_to_add) tree_comp_log(" both with children %lu %lu", cn1->children.size(), cn2->children.size());
                            needs_to_add &= sim > parent_limit;

                            if (needs_to_add) {
                                tree_comp_log(" added");
                                assert(check.find(std::make_pair(cn1->id, cn2->id)) == check.end());
                                check.insert(std::make_pair(cn1->id, cn2->id));
                                p_queue.push(std::make_pair(std::make_pair(cn1, cn2), sim));
                            }
                            tree_comp_log("\n");
                        }
                }

                if ((node1->id != 1) && (!node2->children.empty()) && (node1->level <= node2->level)) {
                    parent_limit = node1->parent->min_chd_similarity * node2-> min_chd_similarity;
                    tree_comp_log("  node1 to node2 children\n");
                    for (auto [ix2, cn2]: node2->children) {
                        sim = Node2NodeCompare(node1, cn2);
                        ++node_calculated;
                        if (sim > max_similarity) max_similarity = sim;
                        if (sim > similarity_for_same) return 2;
                        tree_comp_log("  [%i %i] max sim:%f sim:%f > min_lvl %f %f", node1->id, cn2->id, max_similarity,
                                sim, node1->min_chd_similarity, node2->min_chd_similarity);
                        needs_to_add = (!cn2->children.empty()) && (!node1->children.empty());
                        if (needs_to_add) tree_comp_log(" both with children");
                        needs_to_add &= sim > parent_limit;
                        if (needs_to_add) {
                            tree_comp_log(" added");
                            auto key_ = std::make_pair(node1->id, cn2->id);
                            assert(check.find(key_) == check.end());
                            check.insert(key_);
                            p_queue.push(std::make_pair(std::make_pair(node1, cn2), sim));
                        }
                        tree_comp_log("\n");
                    }
                }

                if (node2->id != 1 && (!node1->children.empty() && (node1->level >= node2->level))) {
                    parent_limit = node2->parent->min_chd_similarity * node1-> min_chd_similarity;
                    tree_comp_log("  node2 to node1 children\n");
                    for (auto [ix1, cn1]: node1->children) {
                        sim = Node2NodeCompare(cn1, node2);
                        ++node_calculated;
                        if (sim > max_similarity) max_similarity = sim;
                        if (sim > similarity_for_same) return 2;
                        tree_comp_log("  [%i %i] max sim:%f sim:%f > min_lvl %f %f", cn1->id, node2->id, max_similarity,
                                sim, node1->min_chd_similarity, node2->min_chd_similarity);
                        needs_to_add = (!node2->children.empty()) && (!cn1->children.empty());
                        if (needs_to_add) tree_comp_log(" both with children");
                        needs_to_add &= sim > parent_limit;

                        if (needs_to_add) {
                            tree_comp_log(" added");
                            assert(check.find(std::make_pair(cn1->id, node2->id)) == check.end());
                            check.insert(std::make_pair(cn1->id, node2->id));
                            p_queue.push(std::make_pair(std::make_pair(cn1, node2), sim));
                        }
                        tree_comp_log("\n");
                    }
                }
            }

//            printf("nodes:%i\n queue:%lu", node_calculated, p_queue.size());
            return max_similarity;
        }

        ~ReidTree() { root_->children.clear(); };

        // outputs to screen tree code in dot format for later look
        [[maybe_unused]] void output_DOT() const {

            std::cout << "graph tree {" << std::endl;
            std::cout.precision(3);
            std::queue<pTNode> q;

            q.push(root_);

            while (!q.empty()) {
                pTNode v = q.front();
                std::cout << v->id << " [ label=\"" << v->id << "[" << v->level << "]:" << v->min_chd_similarity << "-" << v->max_chd_similarity << "\"]"
                          << std::endl;
                if (!v->children.empty()) {
                    for (auto [_, vc]: v->children) {
                        auto cs = vec_to_vec_similarity(v->data, vc->data);
                        std::cout << "    " << v->id << " -- " << vc->id << " [label=\"" << cs << "\"]"
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

//    template<class T, class T_key>
//    class BTree {
//        using TBNode = BNode<T, T_key>;
//        using pTBNode = std::shared_ptr<TBNode>;
//        using pTBNode2 = std::pair<pTBNode, pTBNode>;
//        using queuePairElement = std::pair<pTBNode2, T>;
//        using pairTkey = std::pair<T_key, T_key>;
//    public:
//        T not_to_add = .99;
//        // threshold for queue cleaning
//
//        pTBNode root_ = nullptr;
//        int counter = 0;
//        // to check added Node length and to reserve memory
//        size_t default_vec_len = 0;
//        std::map<std::pair<int, int>, float> cross_cost;
//
//        T Node2NodeCompare(const pTBNode&  n1, const pTBNode&  n2){
//            return vec_to_vec_similarity(n1->data, n2->data);
//            auto ix = std::make_pair(n1->id, n2->id);
//            printf(" Node2NodeCompare<%i %i>", n1->id, n2->id);
//            if (cross_cost.find(ix) == cross_cost.end()) {
//                auto cs = vec_to_vec_similarity(n1->data, n2->data);
//                cross_cost[ix] = cs;
//                printf(" new:%f", cs);
//                return cs;
//            }
//            printf(" old:%f", cross_cost[ix]);
//            return cross_cost[ix];
//
//        }
//
//        void clear() {
//            if (root_ == nullptr) return;
//            root_->clear();
//            counter = 0;
//        }
//
//        // get_object VecParameter return similarity
//        int add_idents_to_tree(std::vector<T> &data_, bool use_max_pass = true) {
//
//            if (root_ == nullptr) {
//                default_vec_len = data_.size();
//                root_ = std::make_shared<TBNode>(++counter, std::vector<T>{});
//                assert(counter == 1);
//            } else
//                assert(data_.size() == default_vec_len);
////            output_DOT();
//            auto insert_node = std::make_shared<TBNode>(++counter, data_);
//            pTBNode pointer = root_;
//            pTBNode old_pointer;
//            T_key nid1{0}, nid2{0};
//            pairTkey pIx{0, 0};
//            T pointer_cross_cs, new_to_left_cs, new_to_right_cs;
//            bool goes_to_child, left_more_similar;
//            add_btree_vector_log("new pointer<%i> root is on <%i>\n", insert_node->id, pointer->id);
//            int stopper = 0;
//            while (pointer != nullptr) { // !cur_node_list->empty()
//                add_btree_vector_log("   look node<%i> pointer to <%i>\n", insert_node->id, pointer->id);
//                // if node is not full
//                if (pointer->left == nullptr) {
//                    pointer->left = insert_node;
//                    pointer->cross_sim = 0;
//                    add_btree_vector_log("   added Node[%i] to Node[%i] as left \n", insert_node->id, pointer->id);
//                    return 1;
//                } else if (pointer->right == nullptr) {
//                    pointer->right = insert_node;
//                    pointer->cross_sim = Node2NodeCompare(pointer->right, insert_node);
//                    add_btree_vector_log("   added Node[%i] to Node[%i] as right with cross_sim: %f\n", insert_node->id, pointer->id, pointer->cross_sim);
//                    return 1;
//                }
//                // push up selected
//                new_to_left_cs = Node2NodeCompare(pointer->left, insert_node);
//                new_to_right_cs = Node2NodeCompare(pointer->right, insert_node);
//                pointer_cross_cs = pointer->cross_sim;
//                goes_to_child = (pointer_cross_cs < new_to_right_cs) && (pointer_cross_cs < new_to_left_cs);
//                left_more_similar = new_to_left_cs > new_to_right_cs;
//                if (goes_to_child && left_more_similar) {
//                    pointer = pointer->left;
//                    add_btree_vector_log("   <%i> pushed to left <%i> cur cross:%f to_let:%f to_right:%f\n", insert_node->id, pointer->id, pointer_cross_cs, new_to_left_cs, new_to_right_cs);
//                    continue;
//                } else if (goes_to_child) {
//                    pointer = pointer->right;
//                    add_btree_vector_log("   <%i> pushed to right <%i> cur cross:%f to_let:%f to_right:%f\n", insert_node->id, pointer->id, pointer_cross_cs, new_to_left_cs, new_to_right_cs);
//                    continue;}
//                else if (left_more_similar) {
//                    add_btree_vector_log("   <%i> is set to left. <%i> pushed up. cross cs: %f -> %f\n", insert_node->id, pointer->left->id, pointer->cross_sim, new_to_right_cs);
//                    pointer->cross_sim = new_to_right_cs;
//                    insert_node.swap(pointer->left);
//                    pointer = pointer->left;
//
//                    pointer->left = insert_node->left;
//                    insert_node->left = nullptr;
//
//                    pointer->right = insert_node->right;
//                    insert_node->right = nullptr;
//
//                    continue;
//                } else {
//                    add_btree_vector_log("   insert_node <%i> is set to pointer <%i> right. <%i> pushed up. cross cs: %f -> %f ", insert_node->id, pointer->id, pointer->right->id, pointer->cross_sim, new_to_left_cs);
//                    pointer->cross_sim = new_to_left_cs;
//                    insert_node.swap(pointer->right);
//                    pointer = pointer->right;
//
//                    pointer->left = insert_node->left;
//                    insert_node->left = nullptr;
//
//                    pointer->right = insert_node->right;
//                    insert_node->right = nullptr;
//
//                    add_btree_vector_log("   next look: pointer <%i> node <%i>\n", pointer->id, insert_node->id);
//                    continue;
//                }
//            }
//            return 1;
//        }
//
//        void pre_compare() {
//            std::queue<pTBNode> queue;
//            queue.push(root_);
//            T_key next_level;
//            pTBNode node;
//            while (!queue.empty()) {
//                node = queue.front();
//                queue.pop();
//                if (!node->isNode()) {
//                    node->cross_sim = 1;
//                    continue;
//                }
//                next_level = node->level + 1;
//                if (node->left) {
//                    node->left->parent = node;
//                    node->left->level = next_level;
//                    queue.push(node->left);
//                }
//                if (node->right) {
//                    node->right->parent = node;
//                    node->right->level = next_level;
//                    queue.push(node->right);
//                }
//            }
//        };
//
//
//#define CD_block \
//        sim = Node2NodeCompare(cur1, cur2); \
//        ++node_calculated;                  \
//        btree_comp_log(" nodes:%i sim:%f max_sim: %f", node_calculated, sim, max_similarity);         \
//        if (sim > max_similarity) max_similarity = sim; \
//        if (sim > similarity_for_same) return 2; \
//        if (sim > parent_limit && cur1->isNode() && cur2->isNode()) {p_queue.push(std::make_pair(std::make_pair(cur1, cur2), sim)); \
//        btree_comp_log(" +[%i %i] parent_limit: %f", cur1->id, cur2->id, parent_limit);         \
//        }
//
////        btree_comp_log("   max sim:%f ")
//        T to_tree(std::shared_ptr<BTree> b) {
//            T similarity_for_same=.95;
//            // fill with roots
//            if (root_ == nullptr || b->root_ == nullptr) return (T) -2;
//            auto q_sort = [](const queuePairElement &a, const queuePairElement &b) { return a.second < b.second; };
//            std::priority_queue<queuePairElement, std::vector<queuePairElement>, decltype(q_sort)> p_queue(q_sort);
//            T max_similarity{0};
//            int node_calculated{0};
//            std::set<std::pair<T_key, T_key>> passed;
//            p_queue.push(std::make_pair(std::make_pair(root_, b->root_), 1));
//            T sim;
//            bool needs_to_add;
//
//            std::set<std::pair<T_key, T_key>> check;
//            pTBNode cur1, cur2;
//
//
//            while (!p_queue.empty()) {
//                queuePairElement v = p_queue.top();
//                auto [node1, node2] = v.first;
//                p_queue.pop();
//
//                btree_comp_log("\ngot [%i(%i) %i(%i)] similarity: %f   calculated:%i queue:%lu crosses:(1) %f (2) %f\n",
//                              node1->id, node1->isNode(), node2->id, node2->isNode(), v.second,
//                            node_calculated, p_queue.size(), node1->cross_sim, node2->cross_sim);
//                auto parent_limit = node1->cross_sim * node2-> cross_sim;
//                if (node1->isNode() && (node2->isNode()) && (node1->level == node2->level)) {
//                    if (node1->left && node2->left) {
//
//                        cur1 = node1->left;
//                        cur2 = node2->left;
//                        btree_comp_log(" node1->left && node2->left <%i %i>", cur1->id, cur2->id);
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//
//                    if (node1->left && node2->right) {
//                        btree_comp_log(" node1->left && node2->right ");
//                        cur1 = node1->left;
//                        cur2 = node2->right;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//
//                    if (node1->right && node2->left) {
//                        btree_comp_log(" node1->right && node2->left ");
//                        cur1 = node1->right;
//                        cur2 = node2->left;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//
//                    if (node1->right && node2->right) {
//                        btree_comp_log(" node1->right && node2->right ");
//                        cur1 = node1->right;
//                        cur2 = node2->right;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//                }
//
//                if ((node1->id != 1) && node2->isNode() && (node1->level <= node2->level)) {
//                    parent_limit = node1->parent->cross_sim * node2->cross_sim;
//                    btree_comp_log("  node1 to node2 children\n");
//                    if (node2->left) {
//                        cur1 = node1;
//                        cur2 = node2->left;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//
//                    if (node2->right) {
//                        cur1 = node1;
//                        cur2 = node2->right;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//                }
//
//                if (node2->id != 1 && (!node1->isNode() && (node1->level >= node2->level))) {
//                    parent_limit = node2->parent->cross_sim * node1-> cross_sim;
//                    btree_comp_log("  node2 to node1 children\n");
//                    if (node1->left) {
//                        cur1 = node1->left;
//                        cur2 = node2;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//
//                    if (node1->right) {
//                        cur1 = node1->right;
//                        cur2 = node2;
//                        CD_block
//                        btree_comp_log("\n");
//                    }
//                }
//            }
//
//            btree_comp_log("nodes:%i\n", node_calculated, p_queue.size());
//            return max_similarity;
//        }
//
//
//        [[maybe_unused]] void output_DOT() const {
//
//            std::cout << "graph tree {" << std::endl;
//            std::cout.precision(3);
//            std::queue<pTBNode> q;
//
//            q.push(root_);
//
//            while (!q.empty()) {
//                pTBNode v = q.front();
//                std::cout << v->id << " [ label=\"" << v->id << "[" << v->level << "]:" << v->cross_sim << "\"]"
//                          << std::endl;
//                if (v->left) {
//                    assert(v != v->left);
//                    auto cs = vec_to_vec_similarity(v->data, v->left->data);
//                    std::cout << "    " << v->id << " -- " << v->left->id << " [label=\"left " << cs << "\"]"
//                              << std::endl;
//                    q.push(v->left);
//                }
//
//                if (v->right) {
//                    assert(v != v->right);
//                    auto cs = vec_to_vec_similarity(v->data, v->right->data);
//                    std::cout << "    " << v->id << " -- " << v->right->id << " [label=\"right " << cs << "\"]"
//                              << std::endl;
//                    q.push(v->right);
//                }
//
//                q.pop();
//            }
//        };
//
//    };
//

}
