#ifndef MISC_H
#define MISC_H

/*#ifdef _WIN32
    #include <windows.h>
    
    static inline double get_time_seconds(void) {
        static LARGE_INTEGER frequency;
        static int initialized = 0;
        LARGE_INTEGER counter;
        
        if (!initialized) {
            QueryPerformanceFrequency(&frequency);
            initialized = 1;
        }
        
        QueryPerformanceCounter(&counter);
        return (double)counter.QuadPart / (double)frequency.QuadPart;
    }
#else
    #include <time.h>
    
    static inline double get_time_seconds(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec + ts.tv_nsec / 1000000000.0;
    }
#endif*/

#include <time.h>

double get_time_seconds(void) {
    return (double)clock() / CLOCKS_PER_SEC;
}

#endif // MISC_H