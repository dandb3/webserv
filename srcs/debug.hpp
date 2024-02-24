#ifndef DEBUG_HPP
#define DEBUG_HPP

#define _CAT(a, b) a ## b
#define CAT(a, b) _CAT(a, b)
#define _NARGS(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define NARGS(...) _NARGS(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define LOG_N(N, ...) CAT(LOG_, N)(__VA_ARGS__)
#define LOG(...)                                \
    do {                                        \
        std::cout << "[DEBUG] ";                \
        LOG_N(NARGS(__VA_ARGS__), __VA_ARGS__); \
        std::cout << std::endl;                 \
    } while (0)
#define LOG_1(_1) \
    std::cout << (_1);
#define LOG_2(_1, _2) \
    LOG_1(_1)LOG_1(_2)
#define LOG_3(_1, _2, _3) \
    LOG_1(_1)LOG_2(_2, _3)
#define LOG_4(_1, _2, _3, _4) \
    LOG_1(_1)LOG_3(_2, _3, _4)
#define LOG_5(_1, _2, _3, _4, _5) \
    LOG_1(_1)LOG_4(_2, _3, _4, _5)
#define LOG_6(_1, _2, _3, _4, _5, _6) \
    LOG_1(_1)LOG_5(_2, _3, _4, _5, _6)
#define LOG_7(_1, _2, _3, _4, _5, _6, _7) \
    LOG_1(_1)LOG_6(_2, _3, _4, _5, _6, _7)
#define LOG_8(_1, _2, _3, _4, _5, _6, _7, _8) \
    LOG_1(_1)LOG_7(_2, _3, _4, _5, _6, _7, _8)
#define LOG_9(_1, _2, _3, _4, _5, _6, _7, _8, _9) \
    LOG_1(_1)LOG_8(_2, _3, _4, _5, _6, _7, _8, _9)
#define LOG_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    LOG_1(_1)LOG_9(_2, _3, _4, _5, _6, _7, _8, _9, _10)

#endif
