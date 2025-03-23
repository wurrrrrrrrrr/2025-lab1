#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define GOLDEN_RATIO_32 0x61C88647
static inline unsigned int hash(unsigned int val, unsigned int bits)
{
    return (val * GOLDEN_RATIO_32) >> (32 - bits);
}
#define MAP_HASH_SIZE(bits) (1 << (bits))
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

struct hlist_head { struct hlist_node *first; };
struct hlist_node { struct hlist_node *next, **pprev; };

typedef struct {
    int bits;
    struct hlist_head *ht;
} map_t;

struct hash_key {
    int key;
    void *data;
    struct hlist_node node;
};

static struct hash_key *find_key(map_t *map, int key)
{
    struct hlist_head *head = &map->ht[hash(key, map->bits)];
    for (struct hlist_node *p = head->first; p; p = p->next) {
        struct hash_key *kn = container_of(p, struct hash_key, node);
        if (kn->key == key)
            return kn;
    }
    return NULL;
}

void *map_get(map_t *map, int key)
{
    struct hash_key *kn = find_key(map, key);
    return kn ? kn->data : NULL;
}

void map_add(map_t *map, int key, void *data)
{
    if (find_key(map, key))
        return;

    struct hash_key *kn = malloc(sizeof(*kn));
    kn->key = key;
    kn->data = data;

    struct hlist_head *h = &map->ht[hash(key, map->bits)];
    struct hlist_node *n = &kn->node, *first = h->first;

    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

map_t *map_init(int bits)
{
    map_t *map = malloc(sizeof(*map));
    map->bits = bits;
    map->ht = calloc(MAP_HASH_SIZE(bits), sizeof(*map->ht));
    return map;
}

void map_deinit(map_t *map)
{
    if (!map) return;

    for (int i = 0; i < MAP_HASH_SIZE(map->bits); i++) {
        struct hlist_head *head = &map->ht[i];
        for (struct hlist_node *p = head->first; p;) {
            struct hash_key *kn = container_of(p, struct hash_key, node);
            struct hlist_node *n = p;
            p = p->next;

            if (n->pprev) {
                struct hlist_node *next = n->next;
                struct hlist_node **pprev = n->pprev;
                *pprev = next;
                if (next)
                    next->pprev = pprev;
                n->next = NULL;
                n->pprev = NULL;
            }

            free(kn->data);
            free(kn);
        }
    }
    free(map->ht);
    free(map);
}

int *twoSum(int *nums, int numsSize, int target, int *returnSize)
{
    map_t *map = map_init(10);
    *returnSize = 0;
    int *ret = malloc(sizeof(int) * 2);
    for (int i = 0; i < numsSize; i++) {
        int *idx = map_get(map, target - nums[i]);
        if (idx) {
            ret[0] = *idx;
            ret[1] = i;
            *returnSize = 2;
            break;
        }
        int *pos = malloc(sizeof(int));
        *pos = i;
        map_add(map, nums[i], pos);
    }
    map_deinit(map);
    return ret;
}

int main(void)
{
    int nums[] = {2, 7, 11, 15};
    int size;
    int *ans = twoSum(nums, 4, 9, &size);
    if (size == 2)
        printf("Found indices: [%d, %d]\n", ans[0], ans[1]);
    else
        printf("No solution\n");
    free(ans);

    return 0;
}
