#ifndef MINI_DETOUR_H
#define MINI_DETOUR_H

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__) || \
    defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)

#define NOMINMAX
#include <Windows.h>

typedef HANDLE threadid_t;
#elif defined(__linux__) || defined(linux)
#include <pthread.h>

typedef pthread_t threadid_t;
#elif defined(__APPLE__)
#include <pthread.h>

typedef pthread_t threadid_t;
#endif

namespace mini_detour
{
    int update_thread(threadid_t thread_id); // Unused for now
    int transaction_begin();
    int transaction_abort();
    int transaction_commit();
    int replace_func(void** ppOriginalFunc, void* _hook); // Will replace the original opcodes without behind able to restore them. Usefull to fill dummy code
    int detour_func(void** ppOriginalFunc, void* _hook); // Will save the original pointer to unhook later
    int unhook_func(void** ppOriginalFunc, void* _hook);
};

#endif // MINI_DETOUR_H

