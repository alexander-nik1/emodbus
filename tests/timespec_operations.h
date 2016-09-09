
#ifndef THE_TIMESPEC_OPERATIONS_H
#define THE_TIMESPEC_OPERATIONS_H

#include <sys/time.h>
#include <time.h>

// A few mathematic operations for timespec type

#ifdef __cplusplus
extern "C" {
#endif

void timespec_print(const struct timespec* _ts);
void timespec_set_sec(struct timespec* _ts, long _sec);
void timespec_set_ms(struct timespec* _ts, long _ms);
void timespec_set_us(struct timespec* _ts, long _us);
void timespec_set_ns(struct timespec* _ts, long _ns);
void timespec_add(const struct timespec* _a, const struct timespec* _b, struct timespec* _result);
void timespec_substract(const struct timespec* _a, const struct timespec* _b, struct timespec* _result);
void timespec_add_sec(struct timespec* _ts, long _sec);
void timespec_add_ms(struct timespec* _ts, long _ms);
void timespec_add_us(struct timespec* _ts, long _us);
void timespec_add_ns(struct timespec* _ts, long _ns);
int timespec_compare(const struct timespec *a, const struct timespec *b);
double timespec_to_double(const struct timespec* _ts);
void double_to_timespec(double _f, struct timespec* _ts);
void timespec_get_clock_realtime(struct timespec *_ts);
void timespec_to_timeval(const struct timespec *_ts, struct timeval *_tv);
void timespec_gettimeofday(struct timespec *_ts);
int timespec_is_past(const struct timespec *_ts);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif // THE_TIMESPEC_OPERATIONS_H
