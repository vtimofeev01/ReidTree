#include "Node.h"


reid_tree::Node::Node(reid_tree::Id id_, reid_tree::VecParameter &data_): n_id(id_) {
    // Init from Vector to array
    // preferred
    n_data.reserve(data_.size());
    for (float & i : data_) n_data.push_back(i);
    n_data_size = (int) data_.size();
}

reid_tree::Node::~Node() = default;
