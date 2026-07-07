#pragma once
#include <format>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <string>
#include <Windows.h>    
#else
#include <iostream>
#endif

// Note: I did not use '#include <cassert>', because it's not consistent around different environments.
//         Also I like my message format more, it's much cleaner.

constexpr inline bool ASSERT_ALWAYS = false;

static void Assert_abort(const char* filename, int line, const char* condition)
{
    // "C:\Users\Marked One\source\repos\LexerTest2\Token.cpp" to just "Token.cpp".
    // It's way cleaner.
    const char* lastSlash1 = std::strrchr(filename, '\\');
    const char* lastSlash2 = std::strrchr(filename, '/');
    const char* lastSlash = (lastSlash1 > lastSlash2) ? lastSlash1 : lastSlash2;
    if (lastSlash) filename = lastSlash + 1;

    const std::string finalMessage = std::format("File: {}\nLine: {}\nCondition: {}", filename, line, condition);

#if defined(_WIN32)
    MessageBoxA(nullptr, finalMessage.c_str(), "Assert Error", MB_OK | MB_ICONERROR);
#else
    std::cerr << '\n' << finalMessage << '\n';
#endif

    std::exit(1);
}
static void Assert_abort(const char* filename, int line, const char* condition, const std::string& message)
{
    // "C:\Users\Marked One\source\repos\LexerTest2\Token.cpp" to just "Token.cpp".
    // It's way cleaner.
    const char* lastSlash1 = std::strrchr(filename, '\\');
    const char* lastSlash2 = std::strrchr(filename, '/');
    const char* lastSlash = (lastSlash1 > lastSlash2) ? lastSlash1 : lastSlash2;
    if (lastSlash) filename = lastSlash + 1;

    const std::string finalMessage = std::format("File: {}\nLine: {}\nCondition: {}\nMessage: {}", filename, line, condition, message);

#if defined(_WIN32)
    MessageBoxA(nullptr, finalMessage.c_str(), "Assert Error", MB_OK | MB_ICONERROR);
#else
    std::cerr << '\n' << finalMessage << '\n';
#endif

    std::exit(1);
}

#ifdef NDEBUG
#define Assert(condition)

#define Assert_Message(condition, message)

#else
#define Assert(condition) \
do { \
    if (not static_cast<bool>(condition)) \
    { \
        Assert_abort(__FILE__, __LINE__, #condition); \
    } \
} while (false)

#define Assert_Message(condition, message) \
do { \
    if (not static_cast<bool>(condition)) \
    { \
        Assert_abort(__FILE__, __LINE__, #condition, message); \
    } \
} while (false)
#endif