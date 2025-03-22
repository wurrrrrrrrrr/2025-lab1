#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>



#ifndef DUDECT_CPUCYCLES_H
#define DUDECT_CPUCYCLES_H

#include <stdint.h>

// http://www.intel.com/content/www/us/en/embedded/training/ia-32-ia-64-benchmark-code-execution-paper.html
static inline int64_t cpucycles(void)
{
#if defined(__i386__) || defined(__x86_64__)
    unsigned int hi, lo;
    __asm__ volatile("rdtsc\n\t" : "=a"(lo), "=d"(hi));
    return ((int64_t) lo) | (((int64_t) hi) << 32);

#elif defined(__aarch64__)
    uint64_t val;
    /* According to ARM DDI 0487F.c, from Armv8.0 to Armv8.5 inclusive, the
     * system counter is at least 56 bits wide; from Armv8.6, the counter
     * must be 64 bits wide.  So the system counter could be less than 64
     * bits wide and it is attributed with the flag 'cap_user_time_short'
     * is true.
     */
    asm volatile("mrs %0, cntvct_el0" : "=r"(val));
    return val;
#else
#error Unsupported Architecture
#endif
}

#endif

#ifndef likely
 #define likely(x) __builtin_expect(!!(x), 1)
 #endif
 
 #ifndef unlikely
 #define unlikely(x) __builtin_expect(!!(x), 0)
 #endif
 
// 宏：從成員指標反推結構體指標
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define list_for_each_safe(node, safe, head)                     \
    for (node = (head)->next, safe = node->next; node != (head); \
         node = safe, safe = node->next)
#define list_for_each(node, head) \
    for (node = (head)->next; node != (head); node = node->next)


#define list_entry(node, type, member) container_of(node, type, member)
// 雙向鏈表節點結構
struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

// 這個結構表示一個鏈表元素，內含一個字串和一個 list_head
typedef struct {
    char *value;
    struct list_head list;
} element_t;

// 初始化雙向鏈表頭
static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

// 在鏈表頭後插入一個節點
static inline void list_add(struct list_head *node, struct list_head *head)
{
    struct list_head *next = head->next;
    next->prev = node;
    node->next = next;
    node->prev = head;
    head->next = node;
}

// 建立一個空的雙向鏈表（隊列）
struct list_head *q_new()
{
    struct list_head *new_qhead = (struct list_head *) malloc(sizeof(struct list_head));
    if (!new_qhead) {
        return NULL;
    }
    INIT_LIST_HEAD(new_qhead);
    return new_qhead;
}

static inline void list_del(struct list_head *node)
{
    struct list_head *next = node->next;
    struct list_head *prev = node->prev;

    next->prev = prev;
    prev->next = next;

#ifdef LIST_POISONING
    node->next = NULL;
    node->prev = NULL;
#endif
}
static inline void list_add_tail(struct list_head *node, struct list_head *head)
{
    struct list_head *prev = head->prev;

    prev->next = node;
    node->next = head;
    node->prev = prev;
    head->prev = node;
}

static inline void list_move_tail(struct list_head *node,
                                  struct list_head *head)
{
    list_del(node);
    list_add_tail(node, head);
}


// 在鏈表頭插入一個新元素，並將字串 s 複製到該元素中
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *new_qelement = (element_t *) malloc(sizeof(element_t));
    if (!new_qelement) {
        return false;
    }
    new_qelement->value = strdup(s);
    if (!new_qelement->value) {
        free(new_qelement);
        return false;
    }
    list_add(&new_qelement->list, head);
    return true;
}


// 遍歷鏈表並印出前 max_print 個元素的字串（用 container_of 取得 element_t 指標）
void print_list(struct list_head *head, int max_print) {
    struct list_head *pos;
    int count = 0;
    for (pos = head->next; pos != head && count < max_print; pos = pos->next) {
        element_t *elem = container_of(pos, element_t, list);
        printf("Element %d: %s\n", count, elem->value);
        count++;
    }
}

void insertion_sort(struct list_head *head) {
    element_t *tp_node, *node;
    struct list_head ans;  // 建立排序用的鏈表頭
    struct list_head *temp, *pos, *ans_pos;

    INIT_LIST_HEAD(&ans);  // 初始化新的排序鏈表

    // 遍歷原本的鏈表，將每個節點移除後插入到排序鏈表 ans 中
    list_for_each_safe(pos, temp, head) {
        tp_node = list_entry(pos, element_t, list);  // 取得節點內容
        list_del(pos);  // 從原鏈表移除

        // 找出在排序鏈表中的正確位置（ans_pos 會指向第一個比 tp_node 大的節點）
        ans_pos = ans.next;
        while (ans_pos != &ans && strcmp(tp_node->value, list_entry(ans_pos, element_t, list)->value) > 0) {
            ans_pos = ans_pos->next;
        }
        // 在 ans_pos 之前插入 tp_node
        list_add(&tp_node->list, ans_pos->prev);
    }

    // 將排序好的 ans 鏈表的節點移回原本的鏈表 head
    INIT_LIST_HEAD(head);
    list_for_each_safe(pos, temp, &ans) {
        node = list_entry(pos, element_t, list);
        list_add_tail(&node->list, head);
    }
}



int main(void)
{
    // 用時間作種子初始化隨機數生成器
    srand((unsigned)time(NULL));

    // 建立空的雙向鏈表
    struct list_head *queue = q_new();
    if (!queue) {
        fprintf(stderr, "Failed to create list.\n");
        return 1;
    }

    int num_elements = 6000;  // 插入 10000 個元素

    for (int i = 0; i < num_elements; i++) {
        char *random_str = calloc(8, sizeof(char));

        int num = rand() % 6000;
        sprintf(random_str, "%d", num);
        if (!q_insert_head(queue, random_str)) {  // 根據需求，這裡也可以呼叫其他函式
            fprintf(stderr, "Failed to insert element.\n");
            free(random_str);
            return 1;
        }
    }
    //print_list(queue, 20);
    int ans = cpucycles();
    insertion_sort(queue);
    //print_list(queue, 20);
    ans = cpucycles() - ans;
    //printf("%d",ans );

    return 0;
}
