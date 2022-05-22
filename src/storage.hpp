#ifndef STORAGE_HPP_
#define STORAGE_HPP_


/**
 * @file
 *
 * This header file contains an implementation of generic key-value storages.
**/
#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#include <optional>
#include <vector>


/**
 * @brief Thread-safe value-type storage for generic types mimics std::atomic.
**/
template <typename V>
class ValueStorage final
{
private:
    V value_;
    std::mutex mutex_;

public:
    ValueStorage();
    ValueStorage(V&& value);
    V load();
    ValueStorage& store(V&& value);
    ValueStorage& store(const V& value);
};

template <typename V>
inline ValueStorage<V>::ValueStorage()
    : value_(), mutex_()
{
}

template <typename V>
inline ValueStorage<V>::ValueStorage(V&& value)
    : value_(std::move(value)), mutex_()
{
}

template <typename V>
inline auto ValueStorage<V>::load() -> V
{
    std::lock_guard lock(mutex_);
    return value_;
}

template <typename V>
inline auto ValueStorage<V>::store(V&& value) -> ValueStorage&
{
    std::lock_guard lock(mutex_);
    value_ = std::move(value);
    return *this;
}

template <typename V>
inline auto ValueStorage<V>::store(const V& value) -> ValueStorage&
{
    std::lock_guard lock(mutex_);
    value_ = value;
    return *this;
}


/**
 * @brief Thread-safe vector-type storage for generic types.
**/
template <typename T>
class VectorStorage final
{
private:
    std::mutex mutex_;
    std::vector<T> vector_;

public:
    VectorStorage();
    void push_back(T&& item);
    void push_back(const T& item);
    std::vector<T> get_last_n(std::size_t n);

    VectorStorage(VectorStorage&&) = delete;
    VectorStorage(const VectorStorage&) = delete;
    VectorStorage& operator=(VectorStorage&&) = delete;
    VectorStorage& operator=(const VectorStorage&) = delete;
};

template <typename T>
inline VectorStorage<T>::VectorStorage()
    : mutex_(), vector_()
{
}

template <typename T>
inline auto VectorStorage<T>::push_back(T&& item) -> void
{
    std::lock_guard lock(mutex_);
    vector_.push_back(std::move(item));
}

template <typename T>
inline auto VectorStorage<T>::push_back(const T& item) -> void
{
    std::lock_guard lock(mutex_);
    vector_.push_back(item);
}

template <typename T>
inline auto VectorStorage<T>::get_last_n(std::size_t n) -> std::vector<T>
{
    std::vector<T> result;
    std::lock_guard lock(mutex_);

    auto size = static_cast<int>(vector_.size());
    auto base = std::max(0, size - static_cast<int>(n));

    for (int i = base; i < size; ++i) {
        result.push_back(vector_[i]);
    }

    return result;
}


/**
 * @brief Thread-safe deque-type storage for generic types.
**/
template <typename T>
class DequeStorage final
{
private:
    std::mutex mutex_;
    std::deque<T> deque_;

public:
    DequeStorage();

    /**
     * @brief Thread-safe check if storage is empty.
    **/
    bool empty();

    /**
     * @brief Thread-safe pop @b optional with value upon success.
    **/
    std::optional<T> maybe_pop();

    /**
     * @brief Thread-safe @b push_back with @b move semantics.
    **/
    DequeStorage& push_back(T&& item);

    /**
     * @brief Thread-safe @b push_back with @b const @b ref semantics.
    **/
    DequeStorage& push_back(const T& item);

    /**
    * @brief Thread-safe @b push_front with @b move semantics.
    **/
    DequeStorage& push_front(T&& item);

    /**
     * @brief Thread-safe @b push_front with @b const @b ref semantics.
    **/
    DequeStorage& push_front(const T& item);

    DequeStorage(DequeStorage&&) = delete;
    DequeStorage(const DequeStorage&) = delete;
    DequeStorage& operator=(DequeStorage&&) = delete;
    DequeStorage& operator=(const DequeStorage&) = delete;
};

template <typename T>
inline DequeStorage<T>::DequeStorage()
    : mutex_(), deque_()
{
}

template <typename T>
inline auto DequeStorage<T>::empty() -> bool
{
    std::lock_guard lock(mutex_);
    return deque_.empty();
}

template <typename T>
inline auto DequeStorage<T>::maybe_pop() -> std::optional<T>
{
    std::optional<T> temp;

    std::lock_guard lock(mutex_);
    if (!deque_.empty()) {
        temp.emplace(std::move(deque_.front()));
        deque_.pop_front();
    }

    return temp;
}

template <typename T>
inline auto DequeStorage<T>::push_back(T&& item) -> DequeStorage<T>&
{
    std::lock_guard lock(mutex_);
    deque_.push_back(std::move(item));
    return *this;
}

template <typename T>
inline auto DequeStorage<T>::push_back(const T& item) -> DequeStorage<T>&
{
    std::lock_guard lock(mutex_);
    deque_.push_back(item);
    return *this;
}

template <typename T>
inline auto DequeStorage<T>::push_front(T&& item) -> DequeStorage<T>&
{
    std::lock_guard lock(mutex_);
    deque_.push_front(std::move(item));
    return *this;
}

template <typename T>
inline auto DequeStorage<T>::push_front(const T& item) -> DequeStorage<T>&
{
    std::lock_guard lock(mutex_);
    deque_.push_front(item);
    return *this;
}


/**
 * @brief Thread-safe key-value storage for generic types.
**/
template <typename K, typename V>
class MapStorage final
{
private:
    std::mutex mutex_;
    std::map<K, V> storage_;

public:
    MapStorage();

    /**
     * @brief Thread-safe value observer. Value itself is not necessarily
     *     thread-safe.
    **/
    V& observe(const K& key);

    /**
     * @brief Thread-safe key collector.
    **/
    std::vector<K> keys();

    MapStorage(MapStorage&&) = delete;
    MapStorage(const MapStorage&) = delete;
    MapStorage& operator=(MapStorage&&) = delete;
    MapStorage& operator=(const MapStorage&) = delete;
};

template <typename K, typename V>
inline MapStorage<K, V>::MapStorage()
    : mutex_(), storage_()
{
}

template <typename K, typename V>
inline auto MapStorage<K, V>::observe(const K& key) -> V&
{
    std::lock_guard lock(mutex_);
    return storage_[key];
}

template <typename K, typename V>
inline auto MapStorage<K, V>::keys() -> std::vector<K>
{
    std::vector<K> result;
    std::lock_guard lock(mutex_);

    for (auto&& [k, v] : storage_) {
        result.emplace_back(k);
    }

    return result;
}


using UserId = std::string;
using Message = std::string;
using UserPair = std::pair<UserId, UserId>;
using PendingDeque = DequeStorage<Message>;
using HistoryVector = VectorStorage<Message>;
using PendingMap = MapStorage<UserId, PendingDeque>;
using HistoryMap = MapStorage<UserPair, HistoryVector>;


/**
 * @brief Thread-safe user information.
**/
class User final
{
private:
    std::atomic_int active_;
    PendingMap pending_;

public:

    /**
     * @brief Default User constructor.
    **/
    User();

    /**
     * @brief Thread-safe try to make user active (online).
    **/
    bool try_acquire(int id);

    /**
     * @brief Thread-safe release of the user if @b id is recognized.
    **/
    void release(int id);

    /**
     * @brief Thread-safe find {user -> opponent} storage with waiting messages.
     *     If mapping does not exist, new storage is created.
    **/
    PendingMap& get_pending();

    User(User&&) = delete;
    User(const User&) = delete;
    User& operator=(User&&) = delete;
    User& operator=(const User&) = delete;
};


using UserMap = MapStorage<UserId, User>;


#endif
