#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <any>
#include <iostream>

class EventBus {
public:
    using EventHandler = std::function<void(const std::any&)>;
    
    // 注册事件监听器（线程安全）
    // template<typename EventType>
    void subscribe(const std::string& event_type, EventHandler handler) {
        std::unique_lock lock(mutex_);
        handlers_map_[event_type].emplace_back(std::move(handler));
    }

    // 发布事件（线程安全）
    template<typename EventType>
    void publish(const std::string& event_type, const EventType& event_data) {
        std::shared_lock lock(mutex_);
        if (auto it = handlers_map_.find(event_type); it != handlers_map_.end()) {
            for (auto& handler : it->second) {
                // 异步执行策略（见后续扩展）
                handler(std::make_any<EventType>(event_data));
            }
        }
    }

    // 移除特定类型所有监听器
    void Unsubscribe(const std::string& event_type) {
        std::unique_lock lock(mutex_);
        handlers_map_.erase(event_type);
    }

private:
    std::unordered_map<std::string, std::vector<EventHandler>> handlers_map_;
    mutable std::shared_mutex mutex_; // 读写锁[8](@ref)
};

#include <thread>
#include <queue>
#include <condition_variable>

class AsyncEventBus : public EventBus {
private:
    AsyncEventBus() : stop_flag_(false) {
    }

    ~AsyncEventBus() {
    }

public:
    static AsyncEventBus& getInstance() {
        static AsyncEventBus bus;
        return bus;
    }

    AsyncEventBus(const AsyncEventBus& bus) = delete;
    AsyncEventBus& operator=(AsyncEventBus& bus) = delete;

    void stop() {
        stop_flag_.store(true);
        cv_.notify_all();
    }

    void start() {
        worker_ = std::thread([this] {
            while (!stop_flag_.load(std::memory_order_relaxed)) {
                std::unique_lock lock(queue_mutex_);
                cv_.wait(lock, [this] { return !event_queue_.empty() || stop_flag_; });
                
                while (!event_queue_.empty()) {
                    auto event = std::move(event_queue_.front());
                    event_queue_.pop();
                    lock.unlock();
                    EventBus::publish(event.type, event.data);
                    lock.lock();
                }
            }
        });
    }

    template<typename EventType>
    void asyncPublish(const std::string& type, EventType data) {
        {
            std::lock_guard lock(queue_mutex_);
            event_queue_.emplace(EventWrapper{type, std::make_any<EventType>(data)});
        }
        cv_.notify_one();
    }



private:
    struct EventWrapper {
        std::string type;
        std::any data;
    };
    
    std::thread worker_;
    std::queue<EventWrapper> event_queue_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_flag_;
};