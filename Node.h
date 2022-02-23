#pragma once

#include "MainDefinitions.h"

namespace reid_tree {
    template <typename T>
    class Node {
    public:
        Id n_id = 0;
        // if cross thr is less than this value - add to children, else - go to nearst child
        T max_node_2_node_difference = DEFAULT_MIN_START_LEVEL;
        std::vector<T> n_data;
        int n_data_size = 0;
        std::vector<Node> children;

        Node(Id id_, std::vector<T>& data_): n_id(id_) {
            n_data_size = data_.size();
            n_data.reserve(n_data_size);
            for (T& i : data_) n_data.push_back(i);
        }
        ~Node()= default;
    };
}
