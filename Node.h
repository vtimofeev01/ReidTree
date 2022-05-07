#pragma once

#include "MainDefinitions.h"

namespace reid_tree {
    template <typename T>
    class Node {
//        using TNode = Node<T>;
//        using pTNode = std::shared_ptr<TNode>;
//        using MapPTNode = std::map<int, pTNode>;
//        using TData =  std::vector<T>;
//        using pairInt = std::pair<int, int>;
//        using mapPairInt = std::map<pairInt, float>;
    public:
        Id n_id;
        std::vector<T> n_data;
        std::map<int, std::shared_ptr<Node<T>>> children;
        Node(Id id_, const std::vector<T>& data_): n_id(id_), n_data(data_) {}
        ~Node()= default;
    };
}
