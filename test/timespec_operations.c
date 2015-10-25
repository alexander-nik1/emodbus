
#include "timespec_operations.h"

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define THOUSAND    (1000L)
#define MILLION     (THOUSAND * THOUSAND)
#define BILLION     (MILLION * THOUSAND)

void timespec_print(const struct timespec* _ts) {
    char sign;
    long sec,nsec;
    assert(_ts != NULL);
    sign = (_ts->tv_sec < 0 || _ts->tv_nsec < 0) ? '-' : ' ';
    sec = _ts->tv_sec >= 0 ? _ts->tv_sec : -_ts->tv_sec;
    nsec = _ts->tv_nsec >= 0 ? _ts->tv_nsec : -_ts->tv_nsec;
    printf("%c%ld.%09ld\n", sign, sec, nsec);
}

void timespec_set_sec(struct timespec* _ts, long _sec) {
    assert(_ts != NULL);
    _ts->tv_sec = _sec;
    _ts->tv_nsec = 0L;
}

void timespec_set_ms(struct timespec* _ts, long _ms) {
    assert(_ts != NULL);
    _ts->tv_sec = _ms / THOUSAND;
    _ts->tv_nsec = (_ms % THOUSAND) * MILLION;
}

void timespec_set_us(struct timespec* _ts, long _us) {
    assert(_ts != NULL);
    _ts->tv_sec = _us / MILLION;
    _ts->tv_nsec = (_us % MILLION) * THOUSAND;
}

void timespec_set_ns(struct timespec* _ts, long _ns) {
    assert(_ts != NULL);
    _ts->tv_sec = _ns / BILLION;
    _ts->tv_nsec = (_ns % BILLION);
}

void timespec_add(const struct timespec* _a, const struct timespec* _b, struct timespec* _result) {
    assert(_a != NULL);
    assert(_b != NULL);
    assert(_result != NULL);
    _result->tv_nsec = _a->tv_nsec + _b->tv_nsec;
    _result->tv_sec = _a->tv_sec + _b->tv_sec;
    if(_result->tv_nsec >= BILLION) {
        _result->tv_nsec -= BILLION;
        ++_result->tv_sec;
    }
//    else if(_result->tv_nsec < 0) { // is this really need here ?
//        if(_result->tv_sec != 0) {
//            _result->tv_nsec += BILLION;
//            --_result->tv_sec;
//        }
//    }
}

// 1427370867,998935404
// 1427370868,179276586
//-2,1819658818

void timespec_substract(const struct timespec* _a, const struct timespec* _b, struct timespec* _result) {
    assert(_a != NULL);
    assert(_b != NULL);
    assert(_result != NULL);
    _result->tv_sec = _a->tv_sec -_b->tv_sec;
    _result->tv_nsec = _a->tv_nsec -_b->tv_nsec;
    if(_result->tv_nsec >= BILLION) {
        _result->tv_nsec -= BILLION;
        ++_result->tv_sec;
    }
    else if(_result->tv_nsec < 0) {
        if(_result->tv_sec != 0) {
            _result->tv_nsec += BILLION;
            --_result->tv_sec;
        }
    }
    else if(_result->tv_sec < 0) {
        _result->tv_nsec = -(BILLION - _result->tv_nsec);
        ++_result->tv_sec;
    }
}

void timespec_add_sec(struct timespec* _ts, long _sec) {
    assert(_ts != NULL);
    _ts->tv_sec += _sec;
    _ts->tv_nsec = 0L;
}

void timespec_add_ms(struct timespec* _ts, long _ms) {
    assert(_ts != NULL);
    _ts->tv_sec += _ms / THOUSAND;
    _ts->tv_nsec += (_ms % THOUSAND) * MILLION;
    if(_ts->tv_nsec >= BILLION) {
        _ts->tv_nsec -= BILLION;
        ++_ts->tv_sec;
    }
}

void timespec_add_us(struct timespec* _ts, long _us) {
    assert(_ts != NULL);
    _ts->tv_sec = _us / MILLION;
    _ts->tv_nsec = (_us % MILLION) * THOUSAND;
    if(_ts->tv_nsec >= BILLION) {
        _ts->tv_nsec -= BILLION;
        ++_ts->tv_sec;
    }
}

void timespec_add_ns(struct timespec* _ts, long _ns) {
    assert(_ts != NULL);
    _ts->tv_sec = _ns / BILLION;
    _ts->tv_nsec = (_ns % BILLION);
    if(_ts->tv_nsec >= BILLION) {
        _ts->tv_nsec -= BILLION;
        ++_ts->tv_sec;
    }
}

int timespec_compare(const struct timespec *a, const struct timespec *b)
{
    if (a->tv_sec!=b->tv_sec)
        return a->tv_sec-b->tv_sec;

    return a->tv_nsec-b->tv_nsec;
}

double timespec_to_double(const struct timespec* _ts) {
    assert(_ts != NULL);
    return (double)(_ts->tv_sec) + (double)(_ts->tv_nsec) / BILLION;
}

void double_to_timespec(double _f, struct timespec* _ts) {
    assert(_ts != NULL);

    _ts->tv_sec = (long)_f;
    _f -= (double)(_ts->tv_sec);
    _ts->tv_nsec = (long)(_f * BILLION);
}

void timespec_get_clock_realtime(struct timespec *_ts) {
    assert(_ts != NULL);
    clock_gettime(CLOCK_REALTIME, _ts);
}

void timespec_to_timeval(const struct timespec *_ts, struct timeval *_tv) {
    assert(_ts != NULL);
    assert(_tv != NULL);
    _tv->tv_sec  = _ts->tv_sec;
    _tv->tv_usec = _ts->tv_nsec / THOUSAND;
}

void timespec_gettimeofday(struct timespec *_ts) {
    struct timeval  tv;
    assert(_ts != NULL);
    gettimeofday(&tv, NULL);
    _ts->tv_sec  = tv.tv_sec;
    _ts->tv_nsec = tv.tv_usec*THOUSAND;
}

int timespec_is_past(const struct timespec *_ts) {
    struct timespec current_time;
    timespec_get_clock_realtime(&current_time);
    return timespec_compare(&current_time, _ts) >= 0;
}
