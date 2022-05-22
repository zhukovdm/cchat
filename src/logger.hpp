#ifndef LOGGER_HPP_
#define LOGGER_HPP_


/**
 * @file
 *
 * This header file declares thread-safe class Logger.
**/
#include <chrono>
#include <mutex>
#include <ostream>
#include <string>
#include <queue>
#include <thread>


/**
 * @brief Thread-safe logger parametrized by a general output stream.
 *     Messages are dumped in batches. Logger shall own passed stream
 *     exclusively!
**/
template <typename T>
class Logger final
{
private:
    static constexpr int64_t LOGGER_FREQ = 100;
    static constexpr std::size_t BATCH_SIZE = 10;

    std::mutex mutex_;
    std::ostream* stream_;
    std::queue<T> queue_;

public:
    Logger(std::ostream* stream);

    /**
     * @brief Thread-safe message logging with move semantics.
    **/
    void log(T&& message);

    /**
     * @brief Thread-safe message logging with const ref. semantics.
    **/
    void log(const T& message);

    /**
     * @brief Thread-safe retrieve input messages from queue in batches and
     *     dumps it into output stream.
     *
     * @param batch_size configures a maximum size of the batch.
    **/
    void loop(const std::atomic_bool& done);

    Logger(Logger&&) = delete;
    Logger(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
};

template <typename T>
inline Logger<T>::Logger(std::ostream* stream)
    : mutex_(), stream_(stream), queue_()
{
}

template <typename T>
inline auto Logger<T>::log(T&& message) -> void
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(message));
}

template <typename T>
inline auto Logger<T>::log(const T& message) -> void
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(message);
}

template <typename T>
inline auto Logger<T>::loop(const std::atomic_bool& done) -> void
{
    for(;;) {
        std::vector<T> buffer;

        // retrieve messages until done and empty
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (done.load() && queue_.empty()) {
                break;
            }

            for (std::size_t i = 0; i < BATCH_SIZE && !queue_.empty(); ++i) {
                buffer.emplace_back(std::move(queue_.front()));
                queue_.pop();
            }
        }

        // dump retrieved messages
        for (auto&& item : buffer) {
            if (stream_->good()) {
                *stream_ << item << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(LOGGER_FREQ));
    }
}


#endif
