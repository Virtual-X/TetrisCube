#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <string>
#include <cstdio>
#include <ctime>

struct TimerClock {
    std::chrono::high_resolution_clock::time_point begin;
    void Start() {
        begin = std::chrono::high_resolution_clock::now();
    }

    void Lap(const std::string& label) {
//		auto elapsed = chrono::high_resolution_clock::to_time_t(begin);
//		printf("Elapsed time for %s: %s", label.c_str(), std::ctime(&elapsed));

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
        printf("Elapsed time for %s: %.3f\n", label.c_str(), elapsed.count() * 1e6);
        fflush(0);
    }
};

#include <time.h>

struct TimerLinux {
    timespec begin;
    void Start() {
        GetTime(begin);
    }

    void Lap(const std::string& label) {
        timespec now;
        GetTime(now);
        now.tv_sec -= begin.tv_sec;
        if (now.tv_nsec < begin.tv_nsec) {
            now.tv_sec--;
            now.tv_nsec += 1000000000LL - begin.tv_nsec;
        } else {
            now.tv_nsec -= begin.tv_nsec;
        }
        printf("Elapsed time for %s: %ld s, %.3f us\n", label.c_str(), now.tv_sec, now.tv_nsec * 1e-3);
        fflush(0);
    }

    void GetTime(timespec& now) {
        clock_gettime(CLOCK_MONOTONIC, &now);
    }
};

typedef TimerLinux Timer;

#endif // TIMER_H
