#ifndef SGX_LIST_H
#define SGX_LIST_H

#ifndef NULL
#define NULL 0
#endif

/// list_head 参（chao）考（xi） linux 内核
struct sgx_list_head
{
    struct sgx_list_head *prev;
    struct sgx_list_head *next;
};

typedef struct sgx_list_head sgx_list_head;

#define INIT_LIST_HEAD(ptr) do {\
    struct sgx_list_head *_ptr = (struct sgx_list_head *)ptr;   \
    (_ptr)->next = (_ptr); (_ptr->prev) = (_ptr);       \
} while(0)


/*
* Insert a new entry to two consecutive entries.
*/
static inline void __list_add(struct sgx_list_head *_new, struct sgx_list_head *prev, struct sgx_list_head *next) {
    _new -> next = next;
    next -> prev = _new;
    prev -> next = _new;
    _new -> prev = prev;
}

static inline void list_add(struct sgx_list_head *_new, struct sgx_list_head *head) {
    __list_add(_new, head, head -> next);
}

static inline void list_add_tail(struct sgx_list_head *_new, struct sgx_list_head *head) {
    __list_add(_new, head -> prev, head);
}

/*
* Delete a entry to two consecutive entries.
*/
static inline void __list_del(struct sgx_list_head *prev, struct sgx_list_head *next) {
    prev -> next = next;
    next -> prev = prev;
}

static inline void list_del(struct sgx_list_head *entry) {
    __list_del(entry -> prev, entry -> next);
    //entry->next = entry->prev = NULL;
}

/*
*   check whether the list is empty
*/
static inline int list_empty(struct sgx_list_head *head) {
    return (head -> next == head) && (head -> prev == head);
}

/// type: sgx_head_list 所在的结构体类型
/// member: 结构体 haede_list_head 成员名
/// 计算 sgx_head_list 在 struct 中的偏移量
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );     \
})

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/*
#define list_for_each_entry(pos, head, member)  \
    for (pos = list_entry((head)->next, typeof(*pos), member);  \
        &(pos->member) != (head);                               \
        pos = list_entry(pos->member.next, typeof(*pos), member))
*/

#endif 
