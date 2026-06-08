# pragma once

namespace zenith
{
    class NonCopyable
    {
        protected:
            NonCopyable() = default;
            ~NonCopyable() = default;
        public:
            NonCopyable(const NonCopyable &other) = delete;
            NonCopyable& operator=(const NonCopyable &other) = delete;
            NonCopyable(NonCopyable&& ) = default;
            NonCopyable& operator=(NonCopyable &&) = default;
    };
}