#pragma once

#include "ReidTree.h"

namespace reid_tree {
    template<typename T, typename T_key>
    class IdentsBase {
        using Ident = std::vector<T>;
        using upIdent = std::unique_ptr<Ident>;
        using Idents = std::deque<upIdent>;
    private:
        bool changed;
        unsigned long ident_size;
    public:
        int store_idents;
        Idents idents;
        std::shared_ptr<reid_tree::ReidTree<T, T_key>> indexed_data;

        explicit IdentsBase(int idents_count = 20) : changed(true), store_idents(idents_count), ident_size(0) {
            indexed_data = std::make_shared<reid_tree::ReidTree<T, T_key>>();
        }

        void add_ident(Ident in_data) {
            changed = true;
            if (ident_size == 0) ident_size = in_data.size(); else assert(ident_size == in_data.size());
            idents.push_back(std::make_unique<Ident>(in_data));
            if (idents.size() > store_idents) idents.pop_front();
        }

        T get_best_match(reid_tree::ReidTree<T, T_key> &indexed_data2) {
            if (changed) {
                changed = false;
                indexed_data.clear();
                for (auto &it: idents) indexed_data.add_idents_to_tree(*it);
                indexed_data.pre_compare();
            }
            return indexed_data.to_tree(indexed_data2);
        }

        T get_best_match(std::shared_ptr<reid_tree::IdentsBase<T, T_key>> &idents_base2) {
            if (changed) {
                changed = false;
                indexed_data->clear();
                for (auto &it: idents) indexed_data->add_idents_to_tree(*it);
                indexed_data->pre_compare();
            }

            if (idents_base2->changed) {
                idents_base2->changed = false;
                idents_base2->indexed_data->clear();
                for (auto &it: idents_base2->idents) idents_base2->indexed_data->add_idents_to_tree(*it);
                idents_base2->indexed_data->pre_compare();
            }
            return indexed_data->to_tree(idents_base2->indexed_data, .95);
        }

    };

}