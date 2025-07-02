#pragma once

#include <chrono>

namespace core
{
    /// @brief High resolution timer with sub-millisecond precision.
    class Timer
    {
    public:
        /// @brief Tick timer, updating stored delta.
        inline void tick();

        /// @brief Reset timer delta.
        inline void reset();

        /// @brief Get the stored time delta.
        /// @return Time delta in milliseconds.
        inline double delta() const;

    private:
        using Clock             = std::chrono::steady_clock;
        using DurationNanos     = std::chrono::duration<double, std::nano>;
        using DurationMillis    = std::chrono::duration<double, std::milli>;
        using TimePoint         = std::chrono::time_point<Clock, DurationNanos>;

        TimePoint m_now = Clock::now();
        TimePoint m_prev = m_now;
    };

    void Timer::tick()
    {
        m_prev = m_now;
        m_now = Clock::now();
    }

    void Timer::reset()
    {
        m_now = Clock::now();
        m_prev = m_now;
    }

    double Timer::delta() const
    {
        return std::chrono::duration_cast<DurationMillis>(m_now - m_prev).count();
    }
} // namespace core
