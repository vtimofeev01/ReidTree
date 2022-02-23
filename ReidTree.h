#pragma once

#include "MainDefinitions.h"
#include "Node.h"

namespace reid_tree {
    class ReidTree {
    public:
        // root will start their childs with this cross
        Similarity start_max_node_2_node_cs_level = DEFAULT_MIN_START_LEVEL;
        // level to level  Node::max_node_2_node_difference change
        Similarity step_node_2_node = DEFAULT_STEP;
        // value when similarity reaches no matter to look later
        Similarity similarity_for_same_person = .95;
        // when similarity reaches this value vector can not be stored
        Similarity not_to_add = 1 - step_node_2_node / 2;
        Node *root_ = nullptr;
        Id counter = 1;
        // to check added Node length and to reserve memory
        unsigned long default_vec_len = VEC_LEN;

        ReidTree();

        static Similarity node_to_node_similarity(Node &a, Node &b);

        static Similarity vec_to_node_similarity(VecParameter &vec, Node &b);

        [[maybe_unused]] static Similarity vec_to_vec_similarity(VecParameter &vec_a, VecParameter &vec_b);

        Id add_vector(VecParameter &data_);

        [[maybe_unused]] Similarity nearst(VecParameter &data_) const;

        Similarity operator&(ReidTree &b);

        ~ReidTree();

        [[maybe_unused]] void output_DOT() const;

    };

}