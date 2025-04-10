#ifndef AUTO_MATCH_H
#define AUTO_MATCH_H

#include <deque>
#include <mutex>
#include "body.h"
#include "common_utils.h"

class AutoPlayerMatcher
{
    private: 
    std::deque<Player> queue;
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

    void enqueue_waitting(Player& p) {
        std::lock_guard<std::mutex> lock(mtx);
        clean_invalid_player(&p);

        p.mills = getMilliseconds(); 
        queue.push_back(p);
    }

    // 传入发起人，返回对手, 如果没匹配到，就把p入队
    Player* auto_match(int level)
    {
        std::lock_guard<std::mutex> lock(mtx);

        Player* p = NULL;
        long mills = getMilliseconds();

        // 先筛选出同级别的，然后再匹配相邻级别
        for (auto it=queue.begin(); it!=queue.end(); it++) {
            if (mills > it->mills + 3000) continue;
            if (it->level != level) continue;
            p = &(*it);
            break;
        }

        if (p == NULL) {
            for (auto it=queue.begin(); it!=queue.end(); it++) {
                if (mills > it->mills + 3000) continue;
                if (it->level == level + 1 || it->level == level - 1) {
                    p = &(*it);
                    break;
                } 
            }
        }

        clean_invalid_player(p);

        return p;
    }

    int clean_invalid_player(Player* deletePlayer) {
        long mills = getMilliseconds();
        for (auto it=queue.begin(); it!=queue.end(); ) {
            if (&(*it) == deletePlayer || mills > it->mills + 3000) {
                queue.erase(it);
            } else {
                it++;
            }
        }
    }
};


#endif // AUTO_MATCH_H