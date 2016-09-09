
#include <emodbus/base/add/s-list.h>

#ifndef NULL
#define NULL ((struct sb_list_head*)0)
#endif

void sb_list_init(struct sb_list_head* _anchor) {
    if(_anchor)
        _anchor->next = NULL;
}

void sb_list_add_to_begin(struct sb_list_head* _anchor,
                          struct sb_list_head* _new_elem) {

    if(_anchor && _new_elem) {
        _new_elem->next = _anchor->next;
        _anchor->next = _new_elem;
    }
}


void sb_list_add_to_end(struct sb_list_head* _anchor,
                        struct sb_list_head* _new_elem) {

    if(_anchor && _new_elem) {
        struct sb_list_head* i = _anchor;

        while(i->next != NULL)
            i = i->next;

        i->next = _new_elem;

        _new_elem->next = NULL;
    }
}

int sb_list_remove(struct sb_list_head* _anchor,
                   struct sb_list_head* _elem) {

    if(_anchor && _elem) {
        struct sb_list_head* i = _anchor;

        while(i->next != _elem && i->next != NULL)
            i = i->next;

        if(i->next) {
            i->next = _elem->next;
            return 0;
        }
        else {
            return -1;
        }
    }
    return -1;
}

int sb_list_has_elem(const struct sb_list_head* _anchor,
                     const struct sb_list_head* _elem) {

    if(_anchor && _elem) {
        const struct sb_list_head* i = _anchor;

        while(i->next != _elem && i->next != NULL)
            i = i->next;

        return i->next != NULL;
    }
    return 0;
}
