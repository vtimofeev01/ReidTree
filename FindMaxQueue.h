#pragma once

#include <vector>

namespace pr_queue {
    typedef float Score;

    template<typename T>
    class FindMaxQueue {

    public:
        Score max_value{0};
        Score delta = .05;
        std::vector<std::pair<Score, T>> data_;

        FindMaxQueue()= default;;
        [[maybe_unused]]  explicit FindMaxQueue(Score radius):delta(radius) {};

        int Push(T data, Score score) {
            int out(1);
            if (score > max_value) { max_value = score;
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
            data_.template emplace_back(score, data);
            return out;
        }

        T get() {
            auto out = data_.front();
            data_.erase(data_.begin());
            return out.second;
        };

        bool empty() {
            return data_.empty();
        };
    };

}
