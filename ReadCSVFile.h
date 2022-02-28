#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "ReidTree.h"
#include <cassert>

namespace reid_tree{

    template<class T>
    std::vector<std::vector<T>> ReadCSVFile(const std::string& fn) {
        std::vector<std::vector<T>> out;
        std::ifstream data(fn);
        unsigned long len = -1;
        if (data.is_open()) {
            std::string line;
            int count = 0;
            while (std::getline(data, line)) {
                std::istringstream ss{line};
                std::string token;
                std::vector<T> reid;
                while (std::getline(ss, token, ',')) {
                    if (!token.empty()) {
                        char *n_str;
                        auto d = std::strtof(token.c_str(), &n_str);
                        if (d == 0 && n_str == token.c_str()) printf("WHAT THE .... %s", token.c_str());
                        reid.push_back(d);
                    }
                }
                if (len == -1) len = reid.size();
                assert(len == reid.size());
                out.push_back(reid);
                count++;
            }
            data.close();
        }
        printf("read %zu from %s\n", out.size(), fn.c_str());
        return out;
    }

    template<class T>
    void VecToTree(ReidTree<T>& tree, const std::vector<std::vector<T>>& vs){
        for (auto &a: vs) tree.add_vector_return_cs(a);
    }

    template<class T>
    ReidTree<T> ReadCSVFileToTree(const std::string& fn) {
        ReidTree<T> out;
        out.similarity_for_same = .94;
        out.not_to_add = .95;
        out.step_node_2_node = .03;
        auto vs = ReadCSVFile<T>(fn);
        VecToTree(out, vs);
        return out;
    }


}