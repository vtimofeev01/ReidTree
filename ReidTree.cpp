#include <cmath>
#include <cassert>
#include <iostream>
#include <queue>
#include <set>
#include <map>
#include <algorithm>
#include "ReidTree.h"
#include "FindMaxQueue.h"


reid_tree::Similarity reid_tree::ReidTree::operator&(ReidTree &b) {
    typedef std::pair<Node *, Node *> pNode2;
    pr_queue::FindMaxQueue<pNode2> q(step_node_2_node * 3);
    Similarity cs;
    int node_calculated{1};
    cs = node_to_node_similarity(*root_, *(b.root_));
    if (cs > similarity_for_same_person) return cs;
    q.Push(std::make_pair(root_, b.root_), cs);
    while (!q.empty()) {
        auto[cn1, cn2] = q.get();
        if (cn1->children.empty() && cn2->children.empty()) {
            return q.max_value;
        } else if (!cn1->children.empty() && !cn2->children.empty()) {
            for (auto &cd1: cn1->children)
                for (auto &cd2: cn2->children) {
                    cs = node_to_node_similarity(cd1, cd2);
                    node_calculated++;
                    if (cs > similarity_for_same_person) return cs;
                    pNode2 new_pair{&cd1, &cd2};
                    q.Push(new_pair, cs);
                }
        } else if (!cn1->children.empty()) {
            for (auto &cd1: cn1->children) {
                cs = node_to_node_similarity(cd1, *cn2);
                node_calculated++;
                if (cs > similarity_for_same_person) return cs;
                pNode2 new_pair{&cd1, cn2};
                q.Push(new_pair, cs);
            }
        } else if (!cn2->children.empty()) {
            for (auto &cd2: cn2->children) {
                cs = node_to_node_similarity(*cn1, cd2);
                node_calculated++;
                if (cs > similarity_for_same_person) return cs;
                pNode2 new_pair{cn1, &cd2};
                q.Push(new_pair, cs);
            }
        }
    }
    return q.max_value;
}

reid_tree::Similarity reid_tree::ReidTree::node_to_node_similarity(Node &a, Node &b) {
    reid_tree::Similarity xx = 0, xy = 0, yy = 0;
    auto size1 = a.n_data_size;
    for (int index = 0; index < size1; index++) {
        xx += std::pow(a.n_data[index], 2.f);
        xy += a.n_data[index] * b.n_data[index];
        yy += std::pow(b.n_data[index], 2.f);
    }
    return (reid_tree::Similarity) xy / ((reid_tree::Similarity) std::sqrt(xx * yy) + 1e-6f);
}

// get VecParameter return 0 - if not needed to add or Node::n_id = 1 ... - if is added
reid_tree::Id reid_tree::ReidTree::add_vector(reid_tree::VecParameter &data_) {
    Similarity max_child_level = start_max_node_2_node_cs_level;
    if (root_ == nullptr) {
        root_ = new Node(counter, data_);
        root_->max_node_2_node_difference = max_child_level;
        default_vec_len = data_.size();
        counter++;
        return counter;
    }
    assert(data_.size() == default_vec_len);
    Node *cur = root_;
    Similarity cur_sim;
    while (cur != nullptr) {
        cur_sim = vec_to_node_similarity(data_, *cur);
        // too similar
        if (cur_sim >= not_to_add) return 0;
        // not too similar but no children
        if (cur->children.empty()) {
            cur->children.emplace_back(counter, data_);
            cur->children.back().max_node_2_node_difference = max_child_level;
            counter++;
            return counter;
        }
        // enumerate vectors
        Similarity sim, max_sim = -1;
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

reid_tree::Similarity reid_tree::ReidTree::vec_to_node_similarity(reid_tree::VecParameter &vec, reid_tree::Node &b) {
    reid_tree::Similarity xx = 0, xy = 0, yy = 0;
    auto size1 = vec.size();
    for (int index = 0; index < size1; index++) {
        xx += std::pow(vec[index], 2.f);
        xy += vec[index] * b.n_data[index];
        yy += std::pow(b.n_data[index], 2.f);
    }
    return (reid_tree::Similarity) xy / ((reid_tree::Similarity) std::sqrt(xx * yy) + 1e-6f);
}

reid_tree::ReidTree::ReidTree() = default;

reid_tree::ReidTree::~ReidTree() {
    delete root_;
}

[[maybe_unused]] reid_tree::Similarity
reid_tree::ReidTree::vec_to_vec_similarity(reid_tree::VecParameter &vec_a, reid_tree::VecParameter &vec_b) {
    reid_tree::Similarity xx = 0, xy = 0, yy = 0;
    auto size1 = vec_a.size();
    for (int index = 0; index < size1; index++) {
        xx += std::pow(vec_a[index], 2.f);
        xy += vec_a[index] * vec_b[index];
        yy += std::pow(vec_b[index], 2.f);
    }
    return (reid_tree::Similarity) xy / ((reid_tree::Similarity) std::sqrt(xx * yy) + 1e-6f);
}

// find nears value by vec/ comments - output data to insert in .DOT to see what was looked
[[maybe_unused]] reid_tree::Similarity reid_tree::ReidTree::nearst(reid_tree::VecParameter &data_) const {
    int nodes_passed = 1;
    pr_queue::FindMaxQueue<reid_tree::Node *> q(step_node_2_node * 2);
    q.Push(root_, vec_to_node_similarity(data_, *root_));
#ifdef RT_OUTPUT_DOT_DATA
    std::vector<unsigned long> best_path;
    std::vector<Node *> all_path;
    best_path.push_back(root_->n_id);
    all_path.push_back(root_);
#endif
    while (!q.empty()) {
        Node *v = q.get();
        if (!v->children.empty()) {
            for (auto &c: v->children) {
                auto cs = vec_to_node_similarity(data_, c);
                auto res = q.Push(&c, cs);
#ifdef RT_OUTPUT_DOT_DATA
                    all_path.push_back(&c);
                    if (res == 2) best_path.push_back(c.n_id);
#endif
                if (cs > similarity_for_same_person) return cs;
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

}

// outputs to screen tree code in dot format for later look
void reid_tree::ReidTree::output_DOT() const {

    std::map<int, std::set<unsigned long>> levels;

    std::cout << "graph tree {" << std::endl;
    std::cout.precision(3);
    std::queue<Node *> q;

    q.push(root_);
    levels[(int) (1000 * (root_->max_node_2_node_difference - step_node_2_node))].insert(root_->n_id);
    while (!q.empty()) {
        auto v = q.front();
        auto lvl = (int) (1000 * v->max_node_2_node_difference);
        if (levels.find(lvl) == levels.end()) levels[lvl] = {};
        if (!v->children.empty()) {
            for (auto &vc: v->children) {
                auto cs = node_to_node_similarity(*v, vc);
                std::cout << "    " << v->n_id << " -- " << vc.n_id << " [label=" << cs << "]" << std::endl;
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

    printf("}");

}
