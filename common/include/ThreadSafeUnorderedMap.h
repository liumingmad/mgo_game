#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <optional>

template <typename Key, typename Value>
class ThreadSafeUnorderedMap
{
public:
    ThreadSafeUnorderedMap() = default;
    ~ThreadSafeUnorderedMap() = default;

    // 插入或更新
    void set(const Key &key, const Value &value)
    {
        std::unique_lock lock(mutex_);
        map_[key] = value;
    }

    // 获取值（返回 std::optional）
    std::optional<Value> get(const Key &key) const
    {
        std::shared_lock lock(mutex_);
        auto it = map_.find(key);
        if (it != map_.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    // 检查是否存在 key
    bool contains(const Key &key) const
    {
        std::shared_lock lock(mutex_);
        return map_.find(key) != map_.end();
    }

    // 删除 key
    void erase(const Key &key)
    {
        std::unique_lock lock(mutex_);
        map_.erase(key);
    }

    // 清空 map
    void clear()
    {
        std::unique_lock lock(mutex_);
        map_.clear();
    }

    // 获取 map 大小
    size_t size() const
    {
        std::shared_lock lock(mutex_);
        return map_.size();
    }

    template <typename Func>
    void for_each(Func func) const
    {
        std::shared_lock lock(mutex_);
        for (const auto &pair : map_)
        {
            func(pair);
        }
    }

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<Key, Value> map_;
};