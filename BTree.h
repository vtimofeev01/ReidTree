#pragma once
#include <memory>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <iostream>
#include <cassert>
//#define btree_comp_log(...)  printf(__VA_ARGS__);
//#define btree_comp_log(...)  //__VA_ARGS__;
#define btree_comp_log(...)  if (false) printf(__VA_ARGS__);
//#define add_btree_vector_log  printf

namespace reid_tree{
    template <typename T, typename TKey, typename TUID>
    class BNode {
    public:
        T cross_sim;
        T cs_left;
        T cs_right;
//        T parent_cross_sim;
        TKey id;
        TKey level;
        TUID uid;
        std::vector<T> data;
        std::shared_ptr<BNode<T, TKey, TUID>> prnt;
        std::shared_ptr<BNode<T, TKey, TUID>> left;
        std::shared_ptr<BNode<T, TKey, TUID>> right;
        BNode(TKey _id, TUID uid_, const std::vector<T>& _data): id(_id), uid(uid_), data(_data), cross_sim(1),
                                                        left(nullptr), right(nullptr), prnt(nullptr),
                                                        level(0), cs_left(-2), cs_right(-2) {};

        void clear(){
            if (left != nullptr) {
                left->clear();
                left = nullptr;
            }
            if (right != nullptr) {
                right->clear();
                right = nullptr;
            }
        }
        bool isNode() {return (left != nullptr) || (right != nullptr);}
        std::string to_str(){
            std::stringstream out;
            out << *this;
            return out.str();
        }
    };

    template <typename T, typename TKey, typename TUID>
    void swap_nodes(std::shared_ptr<BNode<T, TKey, TUID>>& n1, std::shared_ptr<BNode<T, TKey, TUID>>& n2){
        T tmp {n1->cross_sim};
        n1->cross_sim = n2->cross_sim;
        n2->cross_sim = tmp;

        TKey tmp2{n1->level};
        n1->level = n2->level;
        n2->level = tmp2;

        n1->left.swap(n2->left);
        if (n1->left != nullptr) n1->left->prnt = n1;
        if (n2->left != nullptr) n2->left->prnt = n2;
        n1->right.swap(n2->right);
        if (n1->right != nullptr) n1->right->prnt = n1;
        if (n2->right != nullptr) n2->right->prnt = n2;
        n1->prnt.swap(n2->prnt);
        n1.swap(n2);
    }


    template <typename T, typename TKey, typename TUID>
    std::ostream& operator << (std::ostream& os, const BNode<T,TKey,TUID>& nd) {
        os.precision(3);
        os << nd.id << "(uid:" << nd.uid << ", lvl:" << nd.level<< ")";
        os << " left="; if  (nd.left == nullptr) os << "null"; else os << nd.left->id << ":" << nd.cs_left;
        os << " right="; if (nd.right == nullptr) os << "null"; else os << nd.right->id << ":" << nd.cs_right;
        os << " cs:" << nd.cross_sim;
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

        pTBNode root_ = nullptr;
        int counter = -1;
        // to check added Node length and to reserve memory
        size_t default_vec_len = 0;
        mapPairTKeyT cross_cost;
        T add_protector = .99;

        T Node2NodeCompare(const pTBNode&  n1, const pTBNode&  n2){
            return vec_to_vec_similarity(n1->data, n2->data);
        }

        void clear() {
//            printf("before clear:%i\n", size());
            if (root_ == nullptr) return;
            root_->clear();
            root_ = nullptr;
            counter = -1;
//            printf("after clear:%i\n", size());
        }

        // get_object VecParameter return similarity
        int add_idents_to_tree(TUID uid, std::vector<T> &data_, int& calcs, bool use_max_pass = true) {
            if (root_ == nullptr) {
                default_vec_len = data_.size();
                root_ = std::make_shared<TBNode>(++counter, 0, std::vector<T>{});
                root_->level = 0;
                root_->prnt = root_;
                assert(counter == 0);
            } else
                assert(data_.size() == default_vec_len);
//            output_DOT();
//            add_btree_vector_log("\nenter\n");
            auto insert_node = std::make_shared<TBNode>(++counter, uid, std::vector<T>{data_});
//            add_btree_vector_log("new node:%s\n", insert_node->to_str().c_str());
            pTBNode pointer = root_;
            pTBNode old_pointer;
            T pointer_cross_cs, new_to_left_cs, new_to_right_cs;
            bool goes_to_child, left_more_similar;
//            add_btree_vector_log("new pointer<%s> root is on <%s>\n", insert_node->to_str().c_str(), pointer->to_str().c_str());
            int similarity_calcs{0};
            while (pointer != nullptr) { // !cur_node_list->empty()
//                add_btree_vector_log("   insert_node<%s> pointer to <%s> \n", insert_node->to_str().c_str(), pointer->to_str().c_str());
                // if node is not full
                if (pointer->left == nullptr) {
                    insert_node->level = pointer->level + 1;
                    pointer->left = insert_node;
                    insert_node->prnt = pointer;
                    pointer->cross_sim = 1;
//                    add_btree_vector_log("   added Node[%s] to Node[%s] as left \n", insert_node->to_str().c_str(), pointer->to_str().c_str());
                    calcs +=similarity_calcs;
                    return counter;
                } else if (pointer->right == nullptr) {

                    insert_node->level = pointer->level + 1;
                    pointer->right = insert_node;
                    insert_node->prnt = pointer;
                    pointer->cross_sim = Node2NodeCompare(pointer->left, pointer->right);
//                    add_btree_vector_log("   added Node[%s]  to Node[%s] as right with cross_sim: %f\n",
//                                         insert_node->to_str().c_str(), pointer->to_str().c_str(), pointer->cross_sim );
                    calcs += ++similarity_calcs;
                    return counter;
                }
                // push up selected
                new_to_left_cs = Node2NodeCompare(pointer->left, insert_node);
                ++similarity_calcs;
                new_to_right_cs = Node2NodeCompare(pointer->right, insert_node);
                ++similarity_calcs;
                if ((new_to_left_cs > add_protector) || (new_to_right_cs > add_protector)) {return -1; }
                pointer_cross_cs = pointer->cross_sim;
                goes_to_child = (pointer_cross_cs < new_to_right_cs) && (pointer_cross_cs < new_to_left_cs);
                left_more_similar = new_to_left_cs > new_to_right_cs;
//                add_btree_vector_log("    cross:%f left %f right %f to child:%i left_more_similar:%i\n",
//                                     pointer_cross_cs,
//                                     new_to_left_cs,
//                                     new_to_right_cs,
//                                     goes_to_child, left_more_similar);
                if (goes_to_child && left_more_similar) {
                    pointer = pointer->left;
//                    add_btree_vector_log("   <%i> pushed to left <%i> cur cross:%f to_let:%f to_right:%f\n", insert_node->id, pointer->id, pointer_cross_cs, new_to_left_cs, new_to_right_cs);
                    continue;
                }
                else if (goes_to_child) {
                    pointer = pointer->right;
//                    add_btree_vector_log("   <%i> pushed to right <%i> cur cross:%f to_let:%f to_right:%f\n", insert_node->id, pointer->id, pointer_cross_cs, new_to_left_cs, new_to_right_cs);
                    continue;}
                else if (left_more_similar) {
//                    add_btree_vector_log("   <%i> is set to left. <%i> pushed up. cross cs: %f -> %f\n", insert_node->id, pointer->left->id, pointer->cross_sim, new_to_right_cs);
//                    add_btree_vector_log("   insert_node <%s> must be set to pointer <%s> left. <%s> pushed up. cross cs: %f -> %f \n", insert_node->to_str().c_str(), pointer->to_str().c_str(), pointer->left->to_str().c_str(), pointer->cross_sim, new_to_right_cs);
                    pointer->cross_sim = new_to_right_cs;
                    pointer->cs_left = new_to_left_cs;
                    swap_nodes(pointer->left, insert_node);
                    pointer = pointer->left;
                    continue;
                }
                else {
//                    add_btree_vector_log("   insert_node <%s> must be set to pointer <%s> right. <%s> pushed up. cross cs: %f -> %f \n", insert_node->to_str().c_str(), pointer->to_str().c_str(), pointer->right->to_str().c_str(), pointer->cross_sim, new_to_left_cs);
                    pointer->cross_sim = new_to_left_cs;
                    pointer->cs_right = new_to_right_cs;
                    swap_nodes(pointer->right, insert_node);
                    pointer = pointer->right;


//                    add_btree_vector_log("   next look: pointer <%s> node <%s>\n", pointer->to_str().c_str(), insert_node->to_str().c_str());
                    continue;
                }
            }
            calcs += similarity_calcs;
            return counter;
        }


#define CD_block2(XX, YY) \
        sim = Node2NodeCompare(XX, YY); \
        btree_comp_log(" looked %i-%i", (XX)->id, (YY)->id);         \
        ++node_calculated;\
        btree_comp_log(" Nodes:%i%i sim: %f par.lim: %f", (XX)->isNode(), (YY)->isNode(), sim, parent_limit); \
        if (sim > out.similarity) {out.similarity = sim; out.node1 = XX; out.node2 = YY; btree_comp_log(" new_max");} \
        if (sim > similarity_for_same) return out; \
        if (sim > parent_limit * expected_min_sim)  {p_queue.push(std::make_pair(std::make_pair(XX, YY), sim)); \
        btree_comp_log(" added %i %i, queue:%lu", (XX)->id, (YY)->id, p_queue.size());}

        response_t2t<T, T_key, TUID> to_tree(std::shared_ptr<BTree> b, T expected_min_sim) {
            response_t2t<T, T_key, TUID> out{nullptr, nullptr, 0.};
            T similarity_for_same=.95;

            if (root_ == nullptr || b->root_ == nullptr) return out;
//            auto q_sort = [](const queuePairElement &a, const queuePairElement &b) { return a.second < b.second; };
//            std::priority_queue<queuePairElement, std::vector<queuePairElement>, decltype(q_sort)> p_queue(q_sort);
            std::queue<queuePairElement> p_queue;

            T max_similarity{0};
            int node_calculated{0};

            p_queue.push(std::make_pair(std::make_pair(root_, b->root_), 1));

            T sim;
            bool needs_to_add;
            pTBNode cur1, cur2;
            T parent_limit = 0;

            while (!p_queue.empty()) {
                queuePairElement v = p_queue.front();
                auto [node1, node2] = v.first;
                p_queue.pop();
                btree_comp_log("\ngot %i-%i [%s %s] similarity: %f   calculated:%i queue:%lu crosses:(1) %f (2) %f\n",
                               node1->id, node2->id,
                               node1->to_str().c_str(), node2->to_str().c_str(), node2->level, v.second,
                               node_calculated, p_queue.size(), node1->cross_sim, node2->cross_sim);
                parent_limit = node1->cross_sim * node2-> cross_sim;
                if (node1->level == node2->level) {
                    if ((node1->left != nullptr) && (node2->left!= nullptr)) {
                        btree_comp_log(" node1->L <%s> && node2->L <%s>", node1->left->to_str().c_str(), node2->left->to_str().c_str());
                        CD_block2(node1->left, node2->left)
                        btree_comp_log("\n");
                    }

                    if ((node1->left!= nullptr) && (node2->right!= nullptr)) {
                        btree_comp_log(" node1->L && node2->R <%s> ", node2->right->to_str().c_str());
                        CD_block2(node1->left, node2->right)
                        btree_comp_log("\n");
                    }

                    if ((node1->right!= nullptr) && (node2->left!= nullptr)) {
                        btree_comp_log(" node1->R <%s> && node2->L ", node1->right->to_str().c_str());
                        CD_block2(node1->right, node2->left)
                        btree_comp_log("\n");
                    }

                    if ((node1->right!= nullptr) && (node2->right!= nullptr)) {
                        btree_comp_log(" node1->R && node2->R ");
                        CD_block2(node1->right, node2->right)
                        btree_comp_log("\n");
                    }
                }

                if ((node1->level > 0) && (node1->level <= node2->level)) {
                    parent_limit = node1->prnt->cross_sim * node2->cross_sim;
                    if (node2->left != nullptr) {
                        btree_comp_log(" node1 && node2->L ");
                        CD_block2(node1, node2->left)
                        btree_comp_log("\n");
                    }
                    parent_limit = node1->cross_sim * node2->prnt->cross_sim;
                    if (node2->right != nullptr) {
                        btree_comp_log(" node1 && node2->R ");
                        CD_block2(node1, node2->right)
                        btree_comp_log("\n");
                    }
                }

                if ((node2->level > 0) && (node1->level >= node2->level)) {
                    parent_limit = node2->prnt->cross_sim * node1-> cross_sim;
//                    btree_comp_log("  node2 to node1 children\n");
                    if (node1->left != nullptr) {
                        btree_comp_log(" node1->L && node2 ");
                        CD_block2(node1->left, node2)
                        btree_comp_log("\n");
                    }

                    if (node1->right != nullptr) {
                        btree_comp_log(" node1->R && node2 ");
                        CD_block2(node1->right, node2)
                        btree_comp_log("\n");
                    }
                }
            }

            return out;
        }

        int size() {
            int out{0};
            if (!root_) return out;
            pTBNode cur = root_;
            std::queue<pTBNode> q;
            q.push(cur);
            while (! q.empty()) {
                cur = q.front();
                out++;
                if (cur->left != nullptr) q.push(cur->left);
                if (cur->right != nullptr) q.push(cur->right);
                q.pop();
            }
            return out;
        }

        [[maybe_unused]] void output_DOT() const {

            std::cout << "graph tree {" << std::endl;
            std::cout.precision(3);
            std::queue<pTBNode> q;

            q.push(root_);

            while (!q.empty()) {
                pTBNode v = q.front();
                std::cout.precision(3);
                std::cout << v->id << " [ label=\"" << v->to_str().c_str() << "\"]" << std::endl;
//                std::cout << v->id << " [ label=\"" << v->id << "[" << v->level << "]";
//                std::cout  <<" pt:" << v->prnt->cross_sim << " ";
//                std::cout << " cs:" << v->cross_sim << "\"]" << std::endl;

                if (v->left) {
                    assert(v != v->left);
                    assert(v->left->prnt == v);
                    auto cs = vec_to_vec_similarity(v->data, v->left->data);
//                    assert(cs == v->sim_left);
                    std::cout << "    " << v->id << " -- " << v->left->id << " [label=\"left " << cs << "\"]"
                              << std::endl;
                    q.push(v->left);
                }

                if (v->right) {
                    assert(v != v->right);
                    assert(v->right->prnt == v);
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
    public:
        int max_store_idents;
        int erase_store_dents_per_time;
        Idents idents;
        spBTree indexed_data;

        explicit IdentsBBase(int idents_count = 20) : changed(false), erase_store_dents_per_time(idents_count * 2 / 3)
         {
            indexed_data = std::make_shared<BTreeTUID>();
            max_store_idents = idents_count + erase_store_dents_per_time;
//            printf("ident:%i - %i\n", max_store_idents, erase_store_dents_per_time);
        }

        void add_ident(Ident in_data) {
            int c;
            idents.push_back(std::make_unique<Ident>(in_data));
            if (idents.size() > max_store_idents) {
                for (int i = 0; i < erase_store_dents_per_time; i++) { idents.pop_front();}
                changed = true;
            }
            else if (!changed) {
                indexed_data->add_idents_to_tree(0, in_data, c);
            }
        }

        response_t2t<T, T_key, TUID> get_best_match(spIdentsBBase &idents_base2, T expected_min_sim) {
//            printf(" s1 s2 %i %i\n", indexed_data->size(), idents_base2->indexed_data->size());
            int c;
            if (changed) {
                changed = false;
                indexed_data->clear();
//                printf(" changed sz=%lu tree=%i", idents.size(), indexed_data->size());
                for (auto &it: idents) indexed_data->add_idents_to_tree(0, *it, c);
//                printf("->%i", indexed_data->size());
            }

            if (idents_base2->changed) {
                idents_base2->changed = false;
                idents_base2->indexed_data->clear();
                for (auto &it: idents_base2->idents) idents_base2->indexed_data->add_idents_to_tree(0, *it, c);
            }
//            printf("\n s1 s2 %i %i", indexed_data->size(), idents_base2->indexed_data->size());
            return indexed_data->to_tree(idents_base2->indexed_data, expected_min_sim);
        }

    };


}