
#ifndef EMB_COMPATIBLE_H
#define EMB_COMPATIBLE_H

#ifdef __cplusplus
extern "C" {
#endif

struct emb_timer_i {
    void (*start)(void* _user_data, unsigned int _timer_value);
    void (*join)(void* _user_data);
    void (*event)(struct emb_timer_i*);
    void* user_data;
};

struct emb_timed_mutex_i {
    int (*lock_timeout)(void* _user_data, unsigned int _timeout);
    void (*unlock)(void* _user_data);
    void* user_data;
};

#ifdef __cplusplus
}   // extern "C"
#endif

#endif
