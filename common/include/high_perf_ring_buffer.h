#include <atomic>
#include <vector>
#include <cstring>
#include <stdexcept>

template <typename T, bool ThreadSafe = true, size_t Alignment = 64>
class RingBuffer {
private:
    alignas(Alignment) std::vector<T> buffer_;
    alignas(Alignment) std::atomic<size_t> head_{0}; // 生产位置
    alignas(Alignment) std::atomic<size_t> tail_{0}; // 消费位置
    size_t capacity_mask_; // 容量掩码（用于位运算优化）

public:
    explicit RingBuffer(size_t capacity)
        : buffer_(next_power_of_two(capacity)), // 保证容量为2^n
          capacity_mask_(buffer_.size() - 1) {}

    // 动态扩容（可选）
    void reserve(size_t new_capacity) {
        if (new_capacity <= buffer_.size()) return;
        std::vector<T> new_buf(next_power_of_two(new_capacity));
        // 数据迁移逻辑...
    }

    // 批量写入（覆盖策略）
    size_t push_bulk(const T* data, size_t count) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t current_tail = tail_.load(std::memory_order_acquire);
        const size_t free_space = (current_tail - current_head - 1) & capacity_mask_;

        if (count > free_space) {
            // 覆盖最旧数据（Disruptor策略）
            tail_.store((current_tail + (count - free_space)) & capacity_mask_, 
                       std::memory_order_release);
        }

        // 分两段内存拷贝
        const size_t first_chunk = std::min(count, buffer_.size() - (current_head & capacity_mask_));
        std::memcpy(&buffer_[current_head & capacity_mask_], data, first_chunk * sizeof(T));
        if (count > first_chunk) {
            std::memcpy(&buffer_[0], data + first_chunk, (count - first_chunk) * sizeof(T));
        }

        head_.store((current_head + count) & capacity_mask_, std::memory_order_release);
        return count;
    }

    // 批量读取（非破坏式）
    size_t pop_bulk(T* dest, size_t max_count) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t current_head = head_.load(std::memory_order_acquire);
        const size_t available = (current_head - current_tail) & capacity_mask_;

        const size_t to_read = std::min(max_count, available);
        if (to_read == 0) return 0;

        // 分两段内存拷贝
        const size_t first_chunk = std::min(to_read, buffer_.size() - (current_tail & capacity_mask_));
        std::memcpy(dest, &buffer_[current_tail & capacity_mask_], first_chunk * sizeof(T));
        if (to_read > first_chunk) {
            std::memcpy(dest + first_chunk, &buffer_[0], (to_read - first_chunk) * sizeof(T));
        }

        tail_.store((current_tail + to_read) & capacity_mask_, std::memory_order_release);
        return to_read;
    }

    size_t size() const {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t current_tail = tail_.load(std::memory_order_acquire);
        return (current_head - current_tail) & capacity_mask_;
    }

    size_t freespace() const {
        return buffer_.size() - size();
    }

    bool push(const T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);
        size_t free_space = (current_tail - current_head - 1) & capacity_mask_;
    
        // 缓冲区满时覆盖旧数据
        if (free_space < 1) {
            size_t new_tail = (current_tail + (1 - free_space)) & capacity_mask_;
            tail_.store(new_tail, std::memory_order_release);
        }
    
        // 写入元素（无需分段，单元素不跨缓冲区边界）
        size_t pos = current_head & capacity_mask_;
        std::memcpy(&buffer_[pos], &item, sizeof(T));
        
        // 更新head指针
        head_.store(current_head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);
        size_t available = (current_head - current_tail) & capacity_mask_;
    
        if (available == 0) return false;
    
        // 读取元素（无需分段）
        size_t pos = current_tail & capacity_mask_;
        std::memcpy(&item, &buffer_[pos], sizeof(T));
    
        // 更新tail指针
        tail_.store(current_tail + 1, std::memory_order_release);
        return true;
    }

private:
    // 计算最接近的2的幂次方
    static size_t next_power_of_two(size_t n) {
        if (n & (n - 1)) {
            n |= n >> 1; n |= n >> 2; n |= n >> 4;
            n |= n >> 8; n |= n >> 16; return ++n;
        }
        return n;
    }
};