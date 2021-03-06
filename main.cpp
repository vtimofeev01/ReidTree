#include <iostream>
#include <chrono>
#include "ReadCSVFile.h"
#include "ReidTree.h"
#include "BTree.h"
#include <sys/resource.h>

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
//    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_3503_00020.1.csv");
    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_3503_0001.csv");
//    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_5499_0001.csv");
    auto vs2 = reid_tree::ReadCSVFile<FT>("../samples/datas_00020.1.csv");
//    auto vs2 = reid_tree::ReadCSVFile<FT>("../samples/datas_5499_0002.csv");
    begin = std::chrono::steady_clock::now();
    int cnt{0};
    int ix1(0), ix2(0), res1(0), res2(0);
    for (auto &v1: vs1) {
        ix2 = 0;
        for (auto &v2: vs2) {
            auto cs = reid_tree::vec_to_vec_similarity<float>(v1, v2);
            ++cnt;
            if (cs > max_cs) {
                max_cs = cs;
                res1 = ix1;
                res2 = ix2;
//                printf("max is set: %f %i %i\n", max_cs, ix1, ix2);
            }
            ix2++;
        }
        ix1++;
    }
    end = std::chrono::steady_clock::now();
    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "Total cs calcs:" << cnt << " time: " << elapsed_mks.count() << " mks ";
    std::cout << "res:" << max_cs << " ix1:" << res1 << " ix2:" << res2 << std::endl;

    std::vector<float> not_to_adds{.99, 1.1};
    std::vector<float> same_similarity{.9};
    std::vector<int> leaf_size{2, 2, 2, 3,  3, 4, 5};

    printf("\n");

    auto min_et{100000000000};
    max_cs = 0;

    {
        auto bt0 = std::make_shared<reid_tree::BTree<float, int>>();
        auto bt1 = std::make_shared<reid_tree::BTree<float, int>>();
        for (auto nta: not_to_adds)
            for (auto sp: same_similarity)
                for (auto ls: leaf_size) {


                    auto tr0 = std::make_shared<reid_tree::ReidTree<float, int, reid_tree::Node<float, int>>>();
                    auto tr1 = std::make_shared<reid_tree::ReidTree<float, int, reid_tree::Node<float, int>>>();

                    begin = std::chrono::steady_clock::now();
                    tr0->clear();
                    tr1->clear();
                    tr0->not_to_add = nta;
                    tr0->max_node_size = ls;
                    auto tree_size_1 = reid_tree::vector_to_tree<float, int>(tr0, vs1);
                    tr0->pre_compare();

                    tr1->not_to_add = nta;
                    tr1->max_node_size = ls;
                    auto tree_size_2 = reid_tree::vector_to_tree<float, int>(tr1, vs2);
                    tr1->pre_compare();
                    end = std::chrono::steady_clock::now();
                    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                    printf(" leaf tree build: %lu", elapsed_mks.count());
                    int calcs{0};
                    FT cs;

//
                    begin = std::chrono::steady_clock::now();
                    cs = tr0->to_tree(tr1, sp);
                    end = std::chrono::steady_clock::now();
                    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                    printf(" exec: %lu cd:%f", elapsed_mks.count(), cs);
//
//
                    begin = std::chrono::steady_clock::now();
                    bt0->clear();
//                    bt0->not_to_add = 1.99;
                    for (auto v: vs1) bt0->add_idents_to_tree(111, v, calcs);
//                    printf("------------------------1-----------------------------\n\n\n");
//                    bt0->output_DOT();



//                    exit(121);
//                    std::cout << "calcs " << calcs << std::endl;
                    bt1->clear();
//                    bt1->not_to_add = 1.99;
                    for (auto v: vs2) bt1->add_idents_to_tree(0, v, calcs);
//                    std::cout << "calcs " << calcs << std::endl;
//                    printf("------------------------2-----------------------------\n\n\n");
//                    bt1->output_DOT();
//                    bt1->pre_compare();
//                    bt1->output_DOT();

                    end = std::chrono::steady_clock::now();
                    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                    printf(" BT build: %lu", elapsed_mks.count());
//                    bt1->output_DOT();
//                    exit(111);
                    begin = std::chrono::steady_clock::now();
                    auto resp = bt0->to_tree(bt1, .4);
                    cs = resp.similarity;
                    end = std::chrono::steady_clock::now();
                    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                    printf(" exec: %lu cd:%f  ix: %i %i", elapsed_mks.count(), cs, resp.node1->id, resp.node2->id);
//
//                    begin = std::chrono::steady_clock::now();
//                    bt0->clear();
//                    bt0->not_to_add = 1.99;
////                for (auto v: vs1) bt0->add_idents_to_tree_cached(0, v);
//                    for (auto v: vs1) bt0->add_idents_to_tree(0, v, calcs);
//                    bt0->pre_compare();
//                    bt1->clear();
//                    bt1->not_to_add = 1.99;
////                for (auto v: vs2) bt1->add_idents_to_tree_cached(0, v);
//                    for (auto v: vs2) bt1->add_idents_to_tree(0, v, calcs);
//                    bt1->pre_compare();
//                    end = std::chrono::steady_clock::now();
//                    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
//                    printf("    C .99 build: %lu", elapsed_mks.count());
//
//                    begin = std::chrono::steady_clock::now();
//                    cs = bt0->to_tree(bt1).similarity;
//                    end = std::chrono::steady_clock::now();
//                    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
//                    printf(" exec: %lu cd:%f", elapsed_mks.count(), cs);

                    printf("\n");
//                    exit(111);
                }
    }
//    exit(200);
    auto b1 = std::make_shared<reid_tree::IdentsBBase<float, int>>(20);
    auto b2 = std::make_shared<reid_tree::IdentsBBase<float, int>>(20);
//    reid_tree::IdentsBase<float, int> b2(20);
//    exit(100);
    auto trt   = std::make_shared<reid_tree::BTree<float, int>>();
    int calcs2{0};
    for (auto i = 0; i < 15; i++) {
        auto z{0};
        for (auto v: vs1) trt->add_idents_to_tree(++z, v, calcs2);
        printf(" calcs %i size %i\n", calcs2, trt->size());
    }
//    trt->output_DOT();
    for (auto i = 0; i < 33; i++) {

        struct rusage usage{};
        getrusage(RUSAGE_SELF, &usage);
        long new_size = usage.ru_maxrss;
        printf("mem:%lu ", new_size);

        begin = std::chrono::steady_clock::now();
        for (auto &v: vs1) b1->add_ident(v);
        for (auto &v: vs2) b2->add_ident(v);
        end = std::chrono::steady_clock::now();
        elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
        printf(" prep:%lu", elapsed_mks.count());

        printf(" b1 sz=%lu tree=%i", b1->idents.size(), b1->indexed_data->size());
        printf(" b2 sz=%lu tree=%i", b2->idents.size(), b2->indexed_data->size());

        begin = std::chrono::steady_clock::now();
        auto cs1 = b1->get_best_match(b2, .4);
        end = std::chrono::steady_clock::now();
        elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
        printf(" 1st:%lu cs:%f", elapsed_mks.count(), cs1.similarity);

        begin = std::chrono::steady_clock::now();
        auto cs2 = b1->get_best_match(b2, .4);
        end = std::chrono::steady_clock::now();
        elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
        printf(" 2nd:%lu cs:%f", elapsed_mks.count(), cs2.similarity);

        printf("\n");
    }


}