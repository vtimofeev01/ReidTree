#pragma once
#include <cassert>
#include <vector>

namespace reid_tree {

    template<typename T_obj, typename Tv>
    class FindMaxQueue {

    public:
        Tv max_value{0};
        Tv delta = .05;
        std::vector<std::pair<Tv, T_obj>> data_;

        FindMaxQueue()= default;;
        [[maybe_unused]]  explicit FindMaxQueue(Tv radius):delta(radius) {};

        int Push(T_obj data, Tv score) {
            int out(1);
            if (score > max_value) {
                max_value = score;
                out = 2;
            }
            assert(score >= -1.1 && score <= 1.1);
            if (data_.empty()) {
                data_.emplace_back(score, data);
                return out;
            }
            if (max_value - delta > score)  return 0;
            for (auto it = data_.begin(); it != data_.end();){
                if (it->first < max_value - delta )  it = data_.erase(it);
                else ++it;
            }
            data_.emplace_back(score, data);
//            printf("push:%lu score=%f\n", data_.size(), score);
            return out;
        }

        T_obj get_object() {
            auto out = data_.front();
            data_.erase(data_.begin());
            return out.second;
        };

        bool empty() {
            return data_.empty();
        };
    };

}
