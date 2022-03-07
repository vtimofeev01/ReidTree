#pragma once

#include "MainDefinitions.h"

namespace reid_tree {
    template <typename T>
    class Node {
    public:
        Id n_id = 0;
        // if cross thr is less than this value - add to children, else - go to nearst child
        T max_node_2_node_difference = .5;
//        std::vector<T> n_data;
        T *n_data;
        int n_data_size;
        int level{0};
        std::vector<Node> children;

        Node(Id id_, std::vector<T>& data_): n_id(id_), n_data_size(data_.size()) {
            n_data = new T[n_data_size];
            for (size_t i = 0; i <n_data_size; i++)  n_data[i] = data_[i];
        }
        ~Node() {
            delete [] n_data;
        };
    };
}
