
#ifndef THE_CS_CONTAINER_OF_H
#define THE_CS_CONTAINER_OF_H

#ifndef container_of

#define my_offsetof(st, m) ((size_t)(&((st *)0)->m))

#ifdef __cplusplus

#define container_of(_ptr_, _type_, _member_)   \
    ((_type_*)(((char*)_ptr_) - my_offsetof(_type_, _member_)))

#else

#define container_of(ptr, _type_, member) ({      \
    const typeof( ((_type_ *)0)->member )         \
    *__mptr = (ptr);                            \
    (_type_ *)( (char *)__mptr - my_offsetof(_type_,member) );})

#endif

#endif // container_of

#endif // THE_CS_CONTAINER_OF_H
