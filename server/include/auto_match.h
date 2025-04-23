#ifndef AUTO_MATCH_H
#define AUTO_MATCH_H

#include <deque>
#include <mutex>
#include "body.h"
#include "common_utils.h"

class AutoPlayerMatcher
{
    private: 
    std::deque<std::shared_ptr<Player>> queue;
    std::mutex mtx;
    AutoPlayerMatcher() {}
    ~AutoPlayerMatcher() {}

    public:
    static AutoPlayerMatcher& getInstance() {
        static AutoPlayerMatcher p;
        return p;
    }

    AutoPlayerMatcher(const AutoPlayerMatcher&) = delete;
    AutoPlayerMatcher& operator=(const AutoPlayerMatcher&) = delete;

    void enqueue_waitting(std::shared_ptr<Player> self) {
        std::lock_guard<std::mutex> lock(mtx);
        clean_invalid_player(*self);

        self->mills = get_now_milliseconds(); 
        queue.push_back(self);
    }

    // 传入发起人，返回对手, 如果没匹配到，就把p入队
    std::shared_ptr<Player> auto_match(const Player& self)
    {
        std::lock_guard<std::mutex> lock(mtx);

        std::shared_ptr<Player> p;
        long mills = get_now_milliseconds();
        long DURATION = AUTO_MATCH_DURATION * 1000;

        // 先筛选出同级别的，然后再匹配相邻级别
        for (auto it=queue.begin(); it!=queue.end(); it++) {
            std::shared_ptr<Player> one = *it;
            if (mills > one->mills + DURATION) continue;
            if (one->level != self.level) continue;
            if (one->id == self.id) continue; 
            p = one;
            break;
        }

        if (!p) {
            for (auto it=queue.begin(); it!=queue.end(); it++) {
                std::shared_ptr<Player> one = *it;
                if (mills > one->mills + DURATION) continue;
                if (one->level == self.level + 1 || one->level == self.level - 1) {
                    p = one;
                    break;
                } 
            }
        }

        if (p) {
            clean_invalid_player(*p);
        }
        return p;
    }

    int clean_invalid_player(const Player& deletePlayer) {
        long mills = get_now_milliseconds();
        long DURATION = AUTO_MATCH_DURATION * 1000;
        for (auto it=queue.begin(); it!=queue.end(); ) {
            std::shared_ptr<Player> one = *it;
            if (one->id == deletePlayer.id || mills > one->mills + DURATION) {
                queue.erase(it);
            } else {
                it++;
            }
        }
    }
};


#endif // AUTO_MATCH_H