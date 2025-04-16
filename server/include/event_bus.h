#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <any>
#include <iostream>
#include <thread>
#include <queue>
#include <condition_variable>

class EventBus {
public:
    using EventHandler = std::function<void(const std::any&)>;

    template<typename EventType>
    void subscribe(const std::string& event_type, std::function<void(const EventType&)> handler) {
        auto fun = [handler](const std::any& data) {
            try {
                handler(std::any_cast<EventType>(data));
            } catch (const std::bad_any_cast& e) {
                std::cerr << "[EventBus] bad_any_cast: " << e.what() << std::endl;
            }
        };
        std::unique_lock lock(mutex_);
        handlers_map_[event_type].emplace_back(std::move(fun));
    }

    // 发布强类型事件
    template<typename EventType>
    void publish(const std::string& event_type, const EventType& event_data) {
        std::shared_lock lock(mutex_);
        if (auto it = handlers_map_.find(event_type); it != handlers_map_.end()) {
            for (auto& handler : it->second) {
                handler(std::make_any<EventType>(event_data));
            }
        }
    }

    // 内部直接传递 std::any 的事件（给 AsyncEventBus 用）
    void publishAny(const std::string& event_type, const std::any& data) {
        std::shared_lock lock(mutex_);
        if (auto it = handlers_map_.find(event_type); it != handlers_map_.end()) {
            for (auto& handler : it->second) {
                handler(data);
            }
        }
    }

    // 移除事件监听器
    void Unsubscribe(const std::string& event_type) {
        std::unique_lock lock(mutex_);
        handlers_map_.erase(event_type);
    }

protected:
    std::unordered_map<std::string, std::vector<EventHandler>> handlers_map_;
    mutable std::shared_mutex mutex_;
};

// 异步事件总线
class AsyncEventBus : public EventBus {
public:
private:
    AsyncEventBus() : stop_flag_(false) {
    }

    ~AsyncEventBus() {
        stop();
        if (worker_.joinable()) worker_.join();
    }

public:
    static AsyncEventBus& getInstance() {
        static AsyncEventBus bus;
        return bus;
    }

    AsyncEventBus(const AsyncEventBus& bus) = delete;
    AsyncEventBus& operator=(AsyncEventBus& bus) = delete;

    void start() {
        worker_ = std::thread([this] {
            while (!stop_flag_.load(std::memory_order_relaxed)) {
                std::unique_lock lock(queue_mutex_);
                cv_.wait(lock, [this] { return !event_queue_.empty() || stop_flag_; });

                while (!event_queue_.empty()) {
                    auto event = std::move(event_queue_.front());
                    event_queue_.pop();
                    lock.unlock();

                    // 调用 EventBus 的 publishAny，直接传递 std::any
                    publishAny(event.type, event.data);

                    lock.lock();
                }
            }
        });
    }

    void stop() {
        stop_flag_.store(true);
        cv_.notify_all();
    }

    template<typename EventType>
    void asyncPublish(const std::string& type, const EventType& data) {
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