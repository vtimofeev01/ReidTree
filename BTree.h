#pragma once
#include <memory>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <iostream>
#include <cassert>
#define btree_comp_log  //printf
#define add_btree_vector_log printf

namespace reid_tree{
    template <typename T, typename TKey, typename TUID>
    class BNode {
    public:
        T cross_sim;
        TKey id;
        TKey level;
        TUID uid;
        bool mark_deleted;
        std::vector<T> data;
        std::shared_ptr<BNode<T, TKey, TUID>> parent;
        std::shared_ptr<BNode<T, TKey, TUID>> left;
        std::shared_ptr<BNode<T, TKey, TUID>> right;
        BNode(TKey _id, TUID uid_, const std::vector<T>& _data): id(_id), uid(uid_), data(_data), cross_sim(1),
                                                       parent(nullptr), left(nullptr), right(nullptr),
                                                       mark_deleted(false), level(0) {parent = nullptr;};

        void clear(){
            parent.reset();// = nullptr;
            if (left != nullptr) {
                left->clear();
                left.reset();
            }
            if (right != nullptr) {
                right->clear();
                right.reset();
            }
        }
        bool isNode() {return (left != nullptr) || (right != nullptr);}
        std::string to_str(){
            std::stringstream out;
            out << id << "(uid:" << uid << ", lvl:" << level<< ")";
            out << " prnt:"; if (parent == nullptr)out << "null"; else out << std::to_string(parent->id);
            out << " left:"; if (left == nullptr)out << "null"; else out << std::to_string(left->id);
            out << " right:"; if (right == nullptr)out << "null"; else out << std::to_string(right->id);
            out << " cs:" << cross_sim;
            return out.str();
        }
    };

    template <typename T, typename TKey, typename TUID>
    std::ostream& operator << (std::ostream& os, const BNode<T,TKey,TUID>& nd) {
//        os.precision(3);
        os << nd.id << "(uid:" << nd.uid << ", lvl:" << nd.level<< ")";
        os << " prnt:" << (nd.parent == nullptr)? "null": std::to_string(nd.parent->id);
        os << " left:" << (nd.left == nullptr)? "null":std::to_string(nd.left->id);
        os << " right:" << (nd.right == nullptr)? "null":std::to_string(nd.right->id);
        os << "cs:" << nd.cross_sim;
        return os;
    }


    template <typename T, typename TKey, typename TUID>
    struct response_t2t {
        std::shared_ptr<BNode<T, TKey, TUID>> node1;
        std::shared_ptr<BNode<T, TKey, TUID>> node2;
        T similarity;
    };


    template<class T, class TUID, class T_key = int, class TreeNode = BNode<T, T_key, TUID>>
    class BTree {
        using TBNode = TreeNode;
        using pTBNode = std::shared_ptr<TBNode>;
        using pTBNode2 = std::pair<pTBNode, pTBNode>;
        using queuePairElement = std::pair<pTBNode2, T>;
        using pairTkey = std::pair<T_key, T_key>;
        using mapPairTKeyT = std::map<pairTkey, T>;
    public:
        T not_to_add = 1.99;
        // threshold for queue cleaning

        pTBNode root_ = nullptr;
        int counter = 0;
        // to check added Node length and to reserve memory
        size_t default_vec_len = 0;
        mapPairTKeyT cross_cost;

        T Node2NodeCompare(const pTBNode&  n1, const pTBNode&  n2){
            return vec_to_vec_similarity(n1->data, n2->data);
        }

        T Node2NodeCompareCached(const pTBNode&  n1, const pTBNode&  n2, mapPairTKeyT& cache){
            auto ix = std::make_pair(n1->id, n2->id);
            if (cache.find(ix) == cache.end()) {
                auto cs = vec_to_vec_similarity(n1->data, n2->data);
                cache[ix] = cs;
                return cs;
            }
            return cache[ix];
        }

        void clear() {
            if (root_ == nullptr) return;
            root_->clear();
            counter = 0;
        }

        // get_object VecParameter return similarity
        int add_idents_to_tree(TUID uid, std::vector<T> &data_, bool use_max_pass = true) {
            if (root_ == nullptr) {
                default_vec_len = data_.size();
                root_ = std::make_shared<TBNode>(++counter, 0, std::vector<T>{});
                root_->level = 0;
                assert(counter == 1);
            } else
                assert(data_.size() == default_vec_len);
            output_DOT();
            add_btree_vector_log("\nenter\n");
            auto insert_node = std::make_shared<TBNode>(++counter, uid, std::vector<T>{data_});
            add_btree_vector_log("new node:%s\n", insert_node->to_str().c_str());
            pTBNode pointer = root_;
            pTBNode old_pointer;
            T pointer_cross_cs, new_to_left_cs, new_to_right_cs;
            bool goes_to_child, left_more_similar;
            add_btree_vector_log("new pointer<%s> root is on <%s>\n", insert_node->to_str().c_str(), pointer->to_str().c_str());
            int stopper = 0;
            while (pointer != nullptr) { // !cur_node_list->empty()
                add_btree_vector_log("   insert_node<%s> pointer to <%s> \n", insert_node->to_str().c_str(), pointer->to_str().c_str());
                // if node is not full
                if (pointer->left == nullptr) {
                    pointer->left = insert_node;
                    insert_node->parent = pointer;
                    insert_node->level = pointer->level + 1;
                    pointer->cross_sim = 0;
                    add_btree_vector_log("   added Node[%s] to Node[%s] as left \n", insert_node->to_str().c_str(), pointer->to_str().c_str());
                    return counter;
                } else if (pointer->right == nullptr) {
                    pointer->right = insert_node;
                    insert_node->parent = pointer;
                    insert_node->level = pointer->level + 1;
                    pointer->cross_sim = Node2NodeCompare(pointer->left, insert_node);
                    add_btree_vector_log("   added Node[%s]  to Node[%s] as right with cross_sim: %f\n",
                                         insert_node->to_str().c_str(), pointer->to_str().c_str(), pointer->cross_sim );
                    return counter;
                }
                // push up selected
                new_to_left_cs = Node2NodeCompare(pointer->left, insert_node);
                if (new_to_left_cs > not_to_add) return 0;
                new_to_right_cs = Node2NodeCompare(pointer->right, insert_node);
                if (new_to_right_cs > not_to_add) return 0;
                pointer_cross_cs = pointer->cross_sim;
                goes_to_child = (pointer_cross_cs < new_to_right_cs) && (pointer_cross_cs < new_to_left_cs);
                left_more_similar = new_to_left_cs > new_to_right_cs;
                add_btree_vector_log("    cross:%f left %i-%i:%f right %i-%i:%f to child:%i left_more_similar:%i\n",
                                     pointer_cross_cs,
                                     insert_node->id, insert_node->left->id, new_to_left_cs,
                                     insert_node->id, insert_node->right->id, new_to_right_cs,
                                     goes_to_child, left_more_similar);
                if (goes_to_child && left_more_similar) {
                    pointer = pointer->left;
                    add_btree_vector_log("   <%i> pushed to left <%i> cur cross:%f to_let:%f to_right:%f\n", insert_node->id, pointer->id, pointer_cross_cs, new_to_left_cs, new_to_right_cs);
                    continue;
                } else if (goes_to_child) {
                    pointer = pointer->right;
                    add_btree_vector_log("   <%i> pushed to right <%i> cur cross:%f to_let:%f to_right:%f\n", insert_node->id, pointer->id, pointer_cross_cs, new_to_left_cs, new_to_right_cs);
                    continue;}
                else if (left_more_similar) {
//                    add_btree_vector_log("   <%i> is set to left. <%i> pushed up. cross cs: %f -> %f\n", insert_node->id, pointer->left->id, pointer->cross_sim, new_to_right_cs);
                    add_btree_vector_log("   insert_node <%s> must be set to pointer <%s> left. <%s> pushed up. cross cs: %f -> %f \n", insert_node->to_str().c_str(), pointer->to_str().c_str(), pointer->left->to_str().c_str(), pointer->cross_sim, new_to_right_cs);
                    pointer->cross_sim = new_to_right_cs;

                    insert_node->level = pointer->level + 1;
                    insert_node->parent.swap(pointer->left->parent);
                    insert_node->left.swap(pointer->left->left);
                    insert_node->right.swap(pointer->left->right);
                    insert_node->cross_sim = pointer->left->cross_sim;
                    // set child pointers to parent
                    if (insert_node->left != nullptr) insert_node->left->parent = insert_node;
                    if (insert_node->right != nullptr) insert_node->right->parent = insert_node;

                    insert_node.swap(pointer->left);
                    pointer = pointer->left;


                    continue;
                } else {
                    add_btree_vector_log("   insert_node <%s> must be set to pointer <%s> right. <%s> pushed up. cross cs: %f -> %f \n", insert_node->to_str().c_str(), pointer->to_str().c_str(), pointer->right->to_str().c_str(), pointer->cross_sim, new_to_left_cs);
                    pointer->cross_sim = new_to_left_cs;
                    insert_node->level = pointer->level + 1;
                    insert_node->parent.swap(pointer->right->parent);
                    insert_node->left.swap(pointer->right->left);
                    insert_node->right.swap(pointer->right->right);
                    insert_node->cross_sim = pointer->right->cross_sim;
                    // set child pointers to parent
                    if (insert_node->left != nullptr) insert_node->left->parent = insert_node;
                    if (insert_node->right != nullptr) insert_node->right->parent = insert_node;
                    add_btree_vector_log("    - update insert node:%s", insert_node->to_str().c_str());

                    insert_node.swap(pointer->right);
                    pointer = pointer->right;

                    add_btree_vector_log("   next look: pointer <%s> node <%s>\n", pointer->to_str().c_str(), insert_node->to_str().c_str());
                    continue;
                }
            }
            return counter;
        }

        void pre_compare() {};


#define CD_block \
        sim = Node2NodeCompare(cur1, cur2); \
        ++node_calculated;                  \
        btree_comp_log(" nodes:%i sim:%f max_sim: %f", node_calculated, sim, max_similarity);         \
        if (sim > out.similarity) {out.similarity = sim; out.node1 = cur1; out.node2 = cur2;} \
        if (sim > similarity_for_same) return out; \
        if (sim > parent_limit && cur1->isNode() && cur2->isNode()) {p_queue.push(std::make_pair(std::make_pair(cur1, cur2), sim)); \
        btree_comp_log(" +[%i %i] parent_limit: %f", cur1->id, cur2->id, parent_limit);         \
        }

//        btree_comp_log("   max sim:%f ")
        response_t2t<T, T_key, TUID> to_tree(std::shared_ptr<BTree> b) {
            response_t2t<T, T_key, TUID> out{nullptr, nullptr, 0.};
            T similarity_for_same=.95;
            // fill with roots
            if (root_ == nullptr || b->root_ == nullptr) return out;
            auto q_sort = [](const queuePairElement &a, const queuePairElement &b) { return a.second < b.second; };
            std::priority_queue<queuePairElement, std::vector<queuePairElement>, decltype(q_sort)> p_queue(q_sort);
            T max_similarity{0};
            int node_calculated{0};
            std::set<std::pair<T_key, T_key>> passed;
            p_queue.push(std::make_pair(std::make_pair(root_, b->root_), 1));
            T sim;
            bool needs_to_add;

            std::set<std::pair<T_key, T_key>> check;
            pTBNode cur1, cur2;


            while (!p_queue.empty()) {
                queuePairElement v = p_queue.top();
                auto [node1, node2] = v.first;
                p_queue.pop();

                btree_comp_log("\ngot [%i(%i) %i(%i)] similarity: %f   calculated:%i queue:%lu crosses:(1) %f (2) %f\n",
                               node1->id, node1->isNode(), node2->id, node2->isNode(), v.second,
                               node_calculated, p_queue.size(), node1->cross_sim, node2->cross_sim);
                auto parent_limit = node1->cross_sim * node2-> cross_sim;
                if (node1->isNode() && (node2->isNode()) && (node1->level == node2->level)) {
                    if (node1->left && node2->left) {

                        cur1 = node1->left;
                        cur2 = node2->left;
                        btree_comp_log(" node1->left && node2->left <%i %i>", cur1->id, cur2->id);
                        CD_block
                        btree_comp_log("\n");
                    }

                    if (node1->left && node2->right) {
                        btree_comp_log(" node1->left && node2->right ");
                        cur1 = node1->left;
                        cur2 = node2->right;
                        CD_block
                        btree_comp_log("\n");
                    }

                    if (node1->right && node2->left) {
                        btree_comp_log(" node1->right && node2->left ");
                        cur1 = node1->right;
                        cur2 = node2->left;
                        CD_block
                        btree_comp_log("\n");
                    }

                    if (node1->right && node2->right) {
                        btree_comp_log(" node1->right && node2->right ");
                        cur1 = node1->right;
                        cur2 = node2->right;
                        CD_block
                        btree_comp_log("\n");
                    }
                }

                if ((node1->id != 1) && node2->isNode() && (node1->level <= node2->level)) {
                    parent_limit = node1->parent->cross_sim * node2->cross_sim;
                    btree_comp_log("  node1 to node2 children\n");
                    if (node2->left) {
                        cur1 = node1;
                        cur2 = node2->left;
                        CD_block
                        btree_comp_log("\n");
                    }

                    if (node2->right) {
                        cur1 = node1;
                        cur2 = node2->right;
                        CD_block
                        btree_comp_log("\n");
                    }
                }

                if (node2->id != 1 && (!node1->isNode() && (node1->level >= node2->level))) {
                    parent_limit = node2->parent->cross_sim * node1-> cross_sim;
                    btree_comp_log("  node2 to node1 children\n");
                    if (node1->left) {
                        cur1 = node1->left;
                        cur2 = node2;
                        CD_block
                        btree_comp_log("\n");
                    }

                    if (node1->right) {
                        cur1 = node1->right;
                        cur2 = node2;
                        CD_block
                        btree_comp_log("\n");
                    }
                }
            }

            btree_comp_log("nodes:%i\n", node_calculated, p_queue.size());
            return out;
        }

        [[maybe_unused]] void output_DOT() const {

            std::cout << "graph tree {" << std::endl;
            std::cout.precision(3);
            std::queue<pTBNode> q;

            q.push(root_);

            while (!q.empty()) {
                pTBNode v = q.front();
                std::cout << v->id << " [ label=\"" << v->id << "[" << v->level << "]";
                if (v->parent != nullptr) std::cout  <<" pt:" << v->parent->id << " ";
                std::cout << " cs:" << v->cross_sim << "\"]" << std::endl;
                if (v->left) {
                    assert(v != v->left);
                    auto cs = vec_to_vec_similarity(v->data, v->left->data);
                    std::cout << "    " << v->id << " -- " << v->left->id << " [label=\"left " << cs << "\"]"
                              << std::endl;
                    q.push(v->left);
                }

                if (v->right) {
                    assert(v != v->right);
                    auto cs = vec_to_vec_similarity(v->data, v->right->data);
                    std::cout << "    " << v->id << " -- " << v->right->id << " [label=\"right " << cs << "\"]"
                              << std::endl;
                    q.push(v->right);
                }

                q.pop();
            }
        };
    };

    template<typename T, typename TUID, typename T_key = int>
    class IdentsBBase {
        using Ident = std::vector<T>;
        using upIdent = std::unique_ptr<Ident>;
        using Idents = std::deque<upIdent>;
        using BTreeTUID = reid_tree::BTree<T, TUID, T_key>;
        using spBTree =  std::shared_ptr<BTreeTUID>;
        using spIdentsBBase =  std::shared_ptr<IdentsBBase<T,TUID, T_key>>;
    private:
        bool changed;
        unsigned long ident_size;
    public:
        int max_store_idents;
        int erase_store_dents_per_time;
        Idents idents;
        spBTree indexed_data;

        explicit IdentsBBase(int idents_count = 20) : changed(true), erase_store_dents_per_time(5),
        max_store_idents(idents_count + 5), ident_size(0) {
            indexed_data = std::make_shared<BTreeTUID>();
        }

        void add_ident(Ident in_data) {
            idents.push_back(std::make_unique<Ident>(in_data));
            if (idents.size() > max_store_idents) {
                for (int i = 0; i < erase_store_dents_per_time; i++)
                    idents.pop_front();
                changed = true;
            }
            else indexed_data->add_idents_to_tree(0, in_data);
        }

        response_t2t<T, T_key, TUID> get_best_match(spIdentsBBase &idents_base2) {
            if (changed) {
                changed = false;
                indexed_data->clear();
                for (auto &it: idents) indexed_data->add_idents_to_tree(0, *it);
                indexed_data->pre_compare();
            }

            if (idents_base2->changed) {
                idents_base2->changed = false;
                idents_base2->indexed_data->clear();
                for (auto &it: idents_base2->idents) idents_base2->indexed_data->add_idents_to_tree(0, *it);
                idents_base2->indexed_data->pre_compare();
            }
            return indexed_data->to_tree(idents_base2->indexed_data);
        }

    };


}