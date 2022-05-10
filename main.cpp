#include <iostream>
#include <chrono>
#include "ReadCSVFile.h"
#include "ReidTree.h"
#include "IdentsBase.h"
#include "BTree.h"
// dot -Tsvg 2.dot > svg.html

int main() {
    using FT = float;

    auto begin = std::chrono::steady_clock::now();
    auto begin_prep0 = std::chrono::steady_clock::now();
    auto begin_prep1 = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    auto end2 = std::chrono::steady_clock::now();

    auto prep_mks0 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    auto prep_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    auto elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    auto elapsed_mks2 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "The time: " << elapsed_mks.count() << " mks\n";

    FT max_cs{0};
    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_3503_00020.1.csv");
//    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_3503_0001.csv");
//    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_5499_0001.csv");
    auto vs2 = reid_tree::ReadCSVFile<FT>("../samples/datas_00020.1.csv");
//    auto vs2 = reid_tree::ReadCSVFile<FT>("../samples/datas_5499_0002.csv");
    begin = std::chrono::steady_clock::now();
    int cnt{0};
    for (auto &v1: vs1)
        for (auto &v2: vs2) {
            auto cs = reid_tree::vec_to_vec_similarity<float>(v1, v2);
            ++cnt;
            if (cs > max_cs) {
                max_cs = cs;
//                printf("max is set: %f\n", max_cs);
            }
        }
    end = std::chrono::steady_clock::now();
    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "Total cs calcs:" << cnt << " time: " << elapsed_mks.count() << " mks ";
    std::cout << "res:" << max_cs << std::endl;

    std::vector<float> not_to_adds{.99, 1.1};
    std::vector<float> same_similarity{.9};
    std::vector<int> leaf_size{2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13};

    printf("\n");

    auto min_et{100000000000};
    max_cs = 0;

    auto bt0 = std::make_shared<reid_tree::BTree<float, int>>();
    auto bt1 = std::make_shared<reid_tree::BTree<float, int>>();
    for (auto nta: not_to_adds)
        for (auto sp: same_similarity)
            for (auto ls: leaf_size) {

                begin = std::chrono::steady_clock::now();
                auto tr0 = std::make_shared<reid_tree::ReidTree<float, int, reid_tree::Node<float, int>>>();
                auto tr1 = std::make_shared<reid_tree::ReidTree<float, int, reid_tree::Node<float, int>>>();
                tr0->clear();
                tr1->clear();
                tr0->not_to_add = nta;
                tr0->max_node_size = ls;
                auto tree_size_1 = reid_tree::VecToTree<float, int>(tr0, vs1);
                tr0->pre_compare();

                tr1->not_to_add = nta;
                tr1->max_node_size = ls;
                auto tree_size_2 = reid_tree::VecToTree<float, int>(tr1, vs2);
                tr1->pre_compare();
                end = std::chrono::steady_clock::now();
                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                printf(" leaf tree build: %lu", elapsed_mks.count());

                FT cs;
                begin = std::chrono::steady_clock::now();
                cs = tr0->to_tree(tr1, sp);
                end = std::chrono::steady_clock::now();
                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                printf(" exec: %lu cd:%f", elapsed_mks.count(), cs);

                begin = std::chrono::steady_clock::now();
                bt0->clear();
                bt0->not_to_add = 1.99;
                for (auto v: vs1) bt0->add_idents_to_tree(0, v);
//                bt0->output_DOT();
//                exit(100);
                bt0->pre_compare();
//                bt0->output_DOT();
//                exit(111);
                bt1->clear();
                bt1->not_to_add = 1.99;
                for (auto v: vs2) bt1->add_idents_to_tree(0, v);
                bt1->pre_compare();
                end = std::chrono::steady_clock::now();
                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                printf(" BT build: %lu", elapsed_mks.count());

                begin = std::chrono::steady_clock::now();
                cs = bt0->to_tree(bt1);
                end = std::chrono::steady_clock::now();
                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                printf(" exec: %lu cd:%f", elapsed_mks.count(), cs);

                begin = std::chrono::steady_clock::now();
                bt0->clear();
                bt0->not_to_add = .99;
                for (auto v: vs1) bt0->add_idents_to_tree_cached(0, v);
                bt0->pre_compare();
                bt1->clear();
                bt1->not_to_add = .99;
                for (auto v: vs2) bt1->add_idents_to_tree_cached(0, v);
                bt1->pre_compare();
                end = std::chrono::steady_clock::now();
                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                printf("    BT .99 build: %lu", elapsed_mks.count());

                begin = std::chrono::steady_clock::now();
                cs = bt0->to_tree_cached(bt1);
                end = std::chrono::steady_clock::now();
                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                printf(" exec: %lu cd:%f", elapsed_mks.count(), cs);

                printf("\n");

//
//
//
//
//
//                begin = std::chrono::steady_clock::now();
//                prep_mks = std::chrono::duration_cast<std::chrono::microseconds>(begin - begin_prep1);
//
//                FT cs2;
//
//                end = std::chrono::steady_clock::now();
//                elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
//                cs2 = bt0->to_tree(bt1);
//                end2 = std::chrono::steady_clock::now();
//                elapsed_mks2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - end);
//                auto r = 6 * prep_mks0.count() + 9 * elapsed_mks.count();
//                printf("dict(tree_size=%i/%i,  prep_multy=%li time=%li,  total=%li, not_to_add=%f, same=%f, leaf=%i, cs=%f   binary prep=%lu run=%lu cs=%f),\n",
//                       tree_size_1, tree_size_2,
//                       prep_mks0.count(),
//                       elapsed_mks.count(),
//                       r,
//                       tr0->not_to_add,
//                       sp,
//                       ls,
//                       cs,
//                       prep_mks.count(),
//                       elapsed_mks2.count(),
//                       cs2
//                );
//                if (r < min_et) min_et = r;
//                }
//                    tr0->output_DOT();
//                    tr1->output_DOT();
//printf("-----------------------------------");
//                bt0->output_DOT();
//                bt1->output_DOT();
//                exit(200);

            }
    exit(200);
    std::cout << "min time:" << min_et << std::endl;
    auto b1 = std::make_shared<reid_tree::IdentsBase<float, int>>(20);
    auto b2 = std::make_shared<reid_tree::IdentsBase<float, int>>(20);
//    reid_tree::IdentsBase<float, int> b2(20);
    exit(100);
    for (auto i = 0; i < 1000; i++) {
        begin_prep0 = std::chrono::steady_clock::now();
        for (auto &v: vs1) b1->add_ident(v);
        for (auto &v: vs2) b2->add_ident(v);
        begin = std::chrono::steady_clock::now();
        prep_mks0 = std::chrono::duration_cast<std::chrono::microseconds>(begin - begin_prep0);
        auto cs1 = b1->get_best_match(b2);
        end = std::chrono::steady_clock::now();
        elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
        auto cs2 = b1->get_best_match(b2);
        end2 = std::chrono::steady_clock::now();
//        prep_mks = std::chrono::duration_cast<std::chrono::microseconds>(begin - begin_prep);
        elapsed_mks2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - end);
        printf("cs: %f %f t1,2,3 %lu %lu %lu\n", cs1, cs2, prep_mks0.count(), elapsed_mks.count(),
               elapsed_mks2.count());

    }


}