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

    template<class T, class T_key>
    void VecToTree(ReidTree<T, T_key> &tree, std::vector<std::vector<T>> &vs){
        for (auto &v: vs)
            tree.add_idents_to_tree(v);
    }

    template<class T, class T_key>
    int vector_to_tree(std::shared_ptr<ReidTree<T, T_key>> &tree, std::vector<std::vector<T>> &vs){
        int out{0};
        for (auto &v: vs)
            out += tree->add_idents_to_tree(v);
        return out;
    }

    template<class T, class T_key>
    [[maybe_unused]] ReidTree<T, T_key>ReadCSVFileToTree(const std::string& fn) {
        ReidTree<T, T_key> out;
        out.similarity_for_same = .94;
        out.not_to_add = .95;
        out.step_node_2_node = .03;
        auto vs = ReadCSVFile<T>(fn);
        VecToTree<T>(out, vs);
        return out;
    }


}