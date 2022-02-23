#pragma once

#include "MainDefinitions.h"

namespace reid_tree {
    class Node {
    public:
        Id n_id = 0;
        // if cross thr is less than this value - add to children, else - go to nearst child
        Similarity max_node_2_node_difference = DEFAULT_MIN_START_LEVEL;
        reid_tree::VecParameter n_data;
        int n_data_size = 0;
        std::vector<Node> children;

        Node(Id id_, VecParameter &data_);
        ~Node();

    };
}