
#ifndef EMB_CONTAINER_OF_H
#define EMB_CONTAINER_OF_H

#ifndef emb_container_of

#define emb_offsetof(st, m) ((unsigned long)(&((st *)0)->m))

#ifdef __cplusplus

#define emb_container_of(_ptr_, _type_, _member_)   \
    ((_type_*)(((char*)_ptr_) - emb_offsetof(_type_, _member_)))

#else

#define emb_container_of(ptr, _type_, member) ({      \
    const typeof( ((_type_ *)0)->member )         \
    *__mptr = (ptr);                            \
    (_type_ *)( (char *)__mptr - emb_offsetof(_type_,member) );})

#endif

#endif // emb_container_of

#endif // EMB_CONTAINER_OF_H
