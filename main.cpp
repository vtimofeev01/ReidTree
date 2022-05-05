#include <iostream>
#include <chrono>
#include "ReadCSVFile.h"
#include "ReidTree.h"
// dot -Tsvg 1.dot > svg.html

int main() {
    using FT = float;
//    auto rt1 = reid_tree::ReadCSVFileToTree<FT>("../samples/datas_3503_0001.csv");
//    rt1.output_DOT();
//    exit(255);
//    auto rt2 = reid_tree::ReadCSVFileToTree<FT>("../samples/datas_5499_0001.csv");


    auto begin = std::chrono::steady_clock::now();
//    auto r = rt1 & rt2;
    auto end = std::chrono::steady_clock::now();

    auto elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "The time: " << elapsed_mks.count() << " mks\n";

    FT max_cs{0};
    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_3503_0001.csv");
    auto vs2 = reid_tree::ReadCSVFile<FT>("../samples/datas_5499_0001.csv");
    begin = std::chrono::steady_clock::now();
    for (auto &v1: vs1)
        for (auto &v2: vs2) {
            auto cs = reid_tree::vec_to_vec_similarity<float>(v1,v2);
            if (cs > max_cs) {
                max_cs = cs;
//                printf("max is set: %f\n", max_cs);
            }
        }
    end = std::chrono::steady_clock::now();
    elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "The time: " << elapsed_mks.count() << " mks\n";
    std::cout << "res:" << max_cs << std::endl;

    std::vector<float> not_to_adds{ .97};
    std::vector<float> same_similarity{.94};
    std::vector<int> leaf_size{2, 3, 4, 5};

    printf("\n");
    auto tr0 = std::make_shared<reid_tree::ReidTree<float>>();
    auto tr1 = std::make_shared<reid_tree::ReidTree<float>>();
    auto min_et{elapsed_mks};
    max_cs = 0;
    for (auto nta: not_to_adds)
        for (auto sp: same_similarity)
            for (auto ls: leaf_size) {
//                        printf(".\n");
                        tr0->clear();
                        tr1->clear();
                        tr0->not_to_add = nta;
                        tr0->similarity_for_same = sp;
                        reid_tree::VecToTree<float>(tr0, vs1);
                        tr1->not_to_add = nta;
                        tr1->similarity_for_same = sp;
                        reid_tree::VecToTree<float>(tr1, vs2);



                        FT cs;
                        begin = std::chrono::steady_clock::now();

//                        cs = tr0 & vs2;
                        cs = tr0 & tr1;

                        end = std::chrono::steady_clock::now();
                        elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                        auto condition = (cs >= max_cs);
//                        auto condition = (min_et >= elapsed_mks) & (cs >= max_cs);
                        if (cs > max_cs)
                            max_cs = cs;
//                        if (condition)
                        {
                            min_et = elapsed_mks;
                            printf("dict(tree_size=%lu,  time=%li,  not_to_add=%f, same=%f, cs=%f),\n",
                                   tr0->counter,
                                   min_et.count(),
                                   tr0->not_to_add,
                                   tr0->similarity_for_same,
                                   cs
                            );
                        }

//                        exit(200);

                    }

    std::cout << "res:" << max_cs << std::endl;
}