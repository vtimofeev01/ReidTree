#pragma once
#include <memory>

namespace reid_tree {
    template <typename T, typename TKey>
    class Node {
    public:
        T min_lvl;
        T max_lvl;
        TKey n_id;
        TKey lvl;

        std::shared_ptr<Node<T, TKey>> parent;
        std::vector<T> n_data;
        std::map<int, std::shared_ptr<Node<T, TKey>>> children;
        Node(TKey id_, const std::vector<T>& data_): n_id(id_), n_data(data_), min_lvl(2), max_lvl(-2), lvl(0), parent(
                nullptr), children() {}
        ~Node()= default;
    };
}
