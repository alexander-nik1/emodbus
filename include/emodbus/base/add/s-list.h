
#ifndef THE_SINGLE_BIND_LIST_H
#define THE_SINGLE_BIND_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Single-binded list
 *
 * This is a Single - binded list.
 * Very simple, useful and nice thing for
 * programmers :)
 */
struct sb_list_head {
    struct sb_list_head *next;
};

/// Returns an entry for given pointer
#define sb_list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * @brief sb_list_init
 *
 * Initialize an anchor
 *
 * @param[in] _anchor The anchor
 */
void sb_list_init(struct sb_list_head* _anchor);

/**
 * @brief sb_list_add_to_begin
 *
 * Adds element to the begin of list
 *
 * @param[in] _anchor The list anchor
 * @param[in] _new_elem The element to add
 */
void sb_list_add_to_begin(struct sb_list_head* _anchor,
                          struct sb_list_head* _new_elem);

/**
 * @brief sb_list_add_to_end
 *
 * Adds element to the end of list
 *
 * @param[in] _anchor The list anchor
 * @param[in] _new_elem The element to add
 */
void sb_list_add_to_end(struct sb_list_head* _anchor,
                        struct sb_list_head* _new_elem);

/**
 * @brief sb_list_remove
 *
 * Remove an element from list.
 *
 * @param[in] _anchor The list anchor
 * @param[in] _elem The element to remove
 * @return Zero if element was removed, otherwise error code
 */
int sb_list_remove(struct sb_list_head* _anchor,
                   struct sb_list_head* _elem);

/**
 * @brief sb_list_has_elem
 *
 * Distinguish, whether given element is a
 * part of the given list, or not.
 *
 * @param[in] _anchor The list anchor
 * @param[in] _elem The element to find
 * @return 1 if element was found in a list, otherwise - zero.
 */
int sb_list_has_elem(const struct sb_list_head* _anchor,
                     const struct sb_list_head* _elem);

/// Same as sb_list_add_to_begin() call, but before
/// add the element, will be make sure, that this
/// element NOT in this list already.
#define sb_list_add_to_begin_safe(_anchor_, _new_elem_)     \
    if(!sb_list_has_elem(_anchor_, _new_elem_))             \
        sb_list_add_to_begin(_anchor_, _new_elem_);


/// Same as sb_list_add_to_end() call, but before
/// add the element, will be make sure, that this
/// element NOT in this list already.
#define sb_list_add_to_end_safe(_anchor_, _new_elem_)  \
    if(!sb_list_has_elem(_anchor_, _new_elem_))        \
        sb_list_add_to_end(_anchor_, _new_elem_);

/**
 * @brief sb_list_for_each_entry
 *
 * Iterate over list of given type.
 *
 * @param[in] _pos_ The type * to use as a loop counter.
 * @param[in] _anchor_ The list anchor
 */
#define sb_list_for_each(_pos_, _anchor_) \
    for (_pos_ = (_anchor_)->next; _pos_ != NULL; _pos_ = _pos_->next)

/**
 * @brief sb_list_for_each_entry
 *
 * Iterate over list of given type.
 *
 * @param[in] _pos_ The type * to use as a loop counter.
 * @param[in] _type_ Type of a container-struct.
 * @param[in] _anchor_ The list anchor
 * @param[in] _member_ The sb_list_head member name in container-struct.
 */
#define sb_list_for_each_entry(_pos_, _type_, _anchor_, _member_)		\
    for (_pos_ = sb_list_entry((_anchor_)->next, _type_, _member_);     \
         &_pos_->_member_ != NULL;                                      \
         _pos_ = sb_list_entry(_pos_->_member_.next, _type_, _member_))

#ifdef __cplusplus
} // extern "C"
#endif


#endif // THE_SINGLE_BIND_LIST_H
