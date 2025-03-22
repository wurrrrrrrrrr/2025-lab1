#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// CPU cycles 取得函式
static inline int64_t cpucycles(void)
{
#if defined(__i386__) || defined(__x86_64__)
    unsigned int hi, lo;
    __asm__ volatile("rdtsc\n\t" : "=a"(lo), "=d"(hi));
    return ((int64_t) lo) | (((int64_t) hi) << 32);
#elif defined(__aarch64__)
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0" : "=r"(val));
    return val;
#else
#error Unsupported Architecture
#endif
}

// 宏：從成員指標反推結構體指標
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

// 雙向鏈表節點結構
struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

// 鏈表元素結構：包含一個字串與鏈表節點
typedef struct {
    char *value;
    struct list_head list;
} element_t;

// 初始化鏈表頭
static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

// 在鏈表頭後插入節點
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
    struct list_head *new_qhead = malloc(sizeof(struct list_head));
    if (!new_qhead) {
        return NULL;
    }
    INIT_LIST_HEAD(new_qhead);
    return new_qhead;
}

// 刪除鏈表中的節點
static inline void list_del(struct list_head *node)
{
    struct list_head *next = node->next;
    struct list_head *prev = node->prev;
    next->prev = prev;
    prev->next = next;
}

// 判斷鏈表是否為空
static inline int list_empty(const struct list_head *head)
{
    return (head->next == head);
}

// 判斷鏈表是否只有一個節點
static inline int list_is_singular(const struct list_head *head)
{
    return (!list_empty(head) && head->prev == head->next);
}

// 在鏈表頭插入新元素，並複製字串 s
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *new_qelement = malloc(sizeof(element_t));
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

// 遍歷鏈表並印出前 max_print 個元素的字串
void print_list(struct list_head *head, int max_print) {
    struct list_head *pos;
    int count = 0;
    for (pos = head->next; pos != head && count < max_print; pos = pos->next) {
        element_t *elem = container_of(pos, element_t, list);
        printf("Element %d: %s\n", count, elem->value);
        count++;
    }
}

// 沉積排序（以交換數值方式實作）
// 此版本以泡沫排序為基礎，並利用 last 來縮小每輪比較範圍
void sediment_sort(struct list_head *head) {
    if (list_empty(head) || list_is_singular(head)) {
        return;  // 若鏈表為空或只有一個元素，則無需排序
    }
    bool swapped;
    struct list_head *last = head;  // last 為本輪最後比較的節點

    do {
        swapped = false;
        struct list_head *cur = head->next;
        // 當前輪比較範圍為從 head->next 到 last 之前的節點
        while (cur->next != head && cur->next != last) {
            element_t *node1 = container_of(cur, element_t, list);
            element_t *node2 = container_of(cur->next, element_t, list);
            if (atoi(node1->value) > atoi(node2->value)) {
                // 交換兩節點的值
                char *temp = node1->value;
                node1->value = node2->value;
                node2->value = temp;
                swapped = true;
            }
            cur = cur->next;
        }
        last = cur; // 更新 last 為最後一個比較過的節點
    } while (swapped);
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

    int num_elements = 1000;  // 插入 10000 個元素

    for (int i = 0; i < num_elements; i++) {
        char *random_str = calloc(8, sizeof(char));
        int num = rand() % 1000;
        sprintf(random_str, "%d", num);
        if (!q_insert_head(queue, random_str)) {
            fprintf(stderr, "Failed to insert element.\n");
            free(random_str);
            return 1;
        }
    }
    // 可選：印出部分排序前的元素
    // print_list(queue, 20);

    int64_t cycles = cpucycles();
    sediment_sort(queue);
    cycles = cpucycles() - cycles;
    //printf("Sediment_Sort CPU cycles: %ld\n", cycles);

    // 可選：印出部分排序後的元素
    //print_list(queue, 20);

    return 0;
}