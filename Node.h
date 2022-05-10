#pragma once
#include <memory>

namespace reid_tree {
    template <typename T, typename TKey>
    class Node {
    public:
        T min_chd_similarity;
        T max_chd_similarity;
        TKey id;
        TKey level;

        std::shared_ptr<Node<T, TKey>> parent;
        std::vector<T> data;
        std::map<int, std::shared_ptr<Node<T, TKey>>> children;
        Node(TKey id_, const std::vector<T>& data_): id(id_), data(data_), min_chd_similarity(2), max_chd_similarity(-2), level(0), parent(
                nullptr), children() {}
        ~Node()= default;
    };



}
