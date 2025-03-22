#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdint.h>


#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
/*---------------------- 模擬原先的結構定義 ----------------------*/

// 雙向鏈表結構
struct list_head {
    struct list_head *prev;
    struct list_head *next;
};

// 雙向鏈表中的元素結構
typedef struct {
    char *value;
    struct list_head list;  // 用來串接鏈表或做二元樹指標
} element_t;

// 初始化鏈表頭
static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

// 在鏈表尾插入
static inline void list_add_tail(struct list_head *node, struct list_head *head)
{
    struct list_head *prev = head->prev;
    prev->next = node;
    node->next = head;
    node->prev = prev;
    head->prev = node;
}

/*---------------------- CPU cycles ----------------------*/

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

/*---------------------- Tree Sort 相關函式 ----------------------*/

/**
 * tree_insert_node - 將節點插入二元搜尋樹
 * @node: 要插入的節點
 * @root: 指向樹根的指標 (的指標)
 * @s:    字串資料 (若你想在此函式中設定 node->value)
 *
 * 說明：
 * - 利用 element_t->list.prev 當作左子樹指標，
 *   element_t->list.next 當作右子樹指標。
 * - 若 *root 為 NULL，表示這是空樹，將 node 設為根。
 * - 若不為 NULL，根據字串大小插入左或右子樹。
 */
bool tree_insert_node(element_t *node, element_t **root, char *s)
{
    // 如果原本樹是空的，node 成為根節點
    if (*root == NULL) {
        node->list.prev = NULL;
        node->list.next = NULL;
        node->value = s;  // 或者你可以在外部先設定好 node->value
        *root = node;
        return true;
    }
    // 比較字串：若 node->value < (*root)->value 則插入左子樹，否則插右子樹
    if (atoi(s) - atoi((*root)->value) < 0) {
        return tree_insert_node(node, (element_t **) &((*root)->list.prev), s);
    } else {
        return tree_insert_node(node, (element_t **) &((*root)->list.next), s);
    }
}

/**
 * tree_inorder_rebuild - 中序遍歷二元搜尋樹並重建排序後的鏈表
 * @root: 指向樹根的指標
 * @list: 要重建的鏈表頭
 *
 * 說明：
 * - 先遍歷左子樹，再將根節點加入鏈表尾部，最後遍歷右子樹。
 */
void Traverse(element_t *root, struct list_head *list)
{
    if (!root) {
        return;
    }
    // 先記下左右子樹，因為稍後會改動 root->list
    element_t *left = (element_t *) root->list.prev;
    element_t *right = (element_t *) root->list.next;
    
    // 遞迴左子樹
    Traverse(left, list);
    
    // 重新初始化該節點，避免與樹結構衝突
    INIT_LIST_HEAD(&root->list);
    // 將節點插入到鏈表尾
    list_add_tail(&root->list, list);
    
    // 遞迴右子樹
    Traverse(right, list);
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
/*---------------------- Main 測試 ----------------------*/

int main(void)
{
    srand((unsigned)time(NULL));

    // 用來表示整棵二元搜尋樹的根節點指標
    element_t *root = NULL;

    int num_elements = 1000;  // 生成 10000 個元素

    for (int i = 0; i < num_elements; i++) {
        // 1) 建立字串
        char *random_str = calloc(8, sizeof(char));
        if (!random_str) {
            fprintf(stderr, "calloc error\n");
            return 1;
        }
        int num = rand() % 1000;
        sprintf(random_str, "%d", num);

        // 2) 建立新的 element_t
        element_t *new_qelement = malloc(sizeof(element_t));
        if (!new_qelement) {
            fprintf(stderr, "malloc error\n");
            free(random_str);
            return 1;
        }
        // 3) 設定初始值 (左右指標為 NULL)
        new_qelement->list.prev = NULL;
        new_qelement->list.next = NULL;

        // 4) 將節點插入樹中
        //    傳入 &root，以便在 tree_insert_node() 內能改變 root
        bool inserted = tree_insert_node(new_qelement, &root, random_str);
        if (!inserted) {
            fprintf(stderr, "Failed to insert element: %s\n", random_str);
            free(random_str);
            free(new_qelement);
            return 1;
        }
    }

    // 建立一個空的鏈表頭，用來存放排序後的結果
    struct list_head sorted_list;
    INIT_LIST_HEAD(&sorted_list);

    // 以 CPU cycles 計時
    int64_t start = cpucycles();
    // 將整棵樹中序遍歷，重建到 sorted_list
    Traverse(root, &sorted_list);
    int64_t end = cpucycles();

    //printf("Tree sort CPU cycles: %ld\n", end - start);

    // 如果想查看排序後結果，可自行印出部分資料
    // 例如：
    //print_list(&sorted_list, 1000);

    return 0;
}
