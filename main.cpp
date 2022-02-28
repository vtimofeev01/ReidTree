#include <iostream>
#include <chrono>
#include "ReadCSVFile.h"
// dot -Tsvg 1.dot > svg.html

int main() {
    using FT = float;
    auto rt1 = reid_tree::ReadCSVFileToTree<FT>("../samples/datas_3503_0001.csv");
//    rt1.output_DOT();
//    exit(255);
    auto rt2 = reid_tree::ReadCSVFileToTree<FT>("../samples/datas_5499_0001.csv");


    auto begin = std::chrono::steady_clock::now();
//    auto r = rt1 & rt2;
    auto end = std::chrono::steady_clock::now();

    auto elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "The time: " << elapsed_mks.count() << " mks\n";

    FT max_cs{0};
    auto vs1 = reid_tree::ReadCSVFile<FT>("../samples/datas_3503_0001.csv");
    auto vs2 = reid_tree::ReadCSVFile<FT>("../samples/datas_5499_0001.csv");
    begin = std::chrono::steady_clock::now();
    for (const auto &v1: vs1)
        for (const auto &v2: vs2) {
            auto cs = rt1.vec_to_vec_similarity(v1, v2);
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
    std::vector<float> same_similarity{.95, .94, .93};
    std::vector<float> steps{.1, .05, .03, .01};
    std::vector<float> clear_thresholds{.05, .3, .2, .1, .02, .01};
    std::vector<float> start_ntn{.5,.45,  .6,};
    printf("\n");
    auto min_et{elapsed_mks};
    for (auto nta: not_to_adds)
        for (auto sp: same_similarity)
            for (auto step: steps)
                for (auto radius: clear_thresholds)
                    for (auto n2n: start_ntn) {
                        printf(".");
                        reid_tree::ReidTree<float> tr0{};
                        tr0.start_max_node_2_node_cs_level = n2n;
                        tr0.step_node_2_node = step;
                        tr0.not_to_add = nta;
                        tr0.similarity_for_same = sp;
                        tr0.q_clear_thr = radius;
                        reid_tree::VecToTree<float>(tr0, vs1);

                        reid_tree::ReidTree<float> tr1{};
                        tr1.start_max_node_2_node_cs_level = n2n;
                        tr1.step_node_2_node = step;
                        tr1.not_to_add = nta;
                        tr1.similarity_for_same = sp;
                        tr1.q_clear_thr = radius;
                        reid_tree::VecToTree<float>(tr1, vs2);


                        max_cs = 0;
                        FT cs;
                        begin = std::chrono::steady_clock::now();

//                        cs = tr0 & vs2;
                        cs = tr0 & tr1;
                        if (cs > max_cs)
                            max_cs = cs;

                        end = std::chrono::steady_clock::now();
                        elapsed_mks = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
                        if ((min_et > elapsed_mks) & (max_cs > .5))
                        {
                            min_et = elapsed_mks;
                            printf("\ntree size:%lu in %li mks not_to_add:%f same:%f step:%f radius:%f cs:%f start_n2n:%f",
                                   tr0.counter,
                                   min_et.count(),
                                   tr0.not_to_add,
                                   tr0.similarity_for_same,
                                   tr0.step_node_2_node,
                                   tr0.q_clear_thr,
                                   max_cs,
                                   tr0.start_max_node_2_node_cs_level
                            );
                        }
//                        exit(200);

                    }

    std::cout << "res:" << max_cs << std::endl;
}