#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


typedef struct block {
    size_t size;
    struct block *l, *r;
} block_t;

//block_t **find_free_tree(block_t **root, block_t *target);
//block_t *find_predecessor_free_tree(block_t **root, block_t *node);

/* Function to find the pointer to a node in the tree. */
block_t **find_free_tree(block_t **root, block_t *target) {
    block_t **current = root;
    while (*current) {
        if ((*current)->size == target->size) {
            return current;
        } else if (target->size < (*current)->size) {
            current = &(*current)->l;
        } else {
            current = &(*current)->r;
        }
    }
    return NULL;
}

/* Function to find the in-order predecessor of a node in the tree. */
//block_t *find_predecessor_free_tree(block_t **root, block_t *node) {
//    block_t *current = node->l;
//    while (current && current->r) {
//        current = current->r;  // Traverse right to find the rightmost node.
//    }
//    return current;
//}
/*
 * Structure representing a free memory block in the memory allocator.
 * The free tree is a binary search tree that organizes free blocks (of type block_t)
 * to efficiently locate a block of appropriate size during memory allocation.
 */
void remove_free_tree(block_t **root, block_t *target)
{
    /* Locate the pointer to the target node in the tree. */
    block_t **node_ptr = find_free_tree(root, target);

    /* If the target node has two children, we need to find a replacement. */
    if ((*node_ptr)->l && (*node_ptr)->r) {
        /* Find the in-order predecessor:
         * This is the rightmost node in the left subtree.
         */
        block_t **pred_ptr = &(*node_ptr)->l;
        while ((*pred_ptr)->r)
            pred_ptr = &(*pred_ptr)->r;

        /* Verify the found predecessor using a helper function (for debugging).
         */
        //block_t *expected_pred = find_predecessor_free_tree(root, *node_ptr);
        //assert(expected_pred == *pred_ptr);

        /* If the predecessor is the immediate left child. */
        if (*pred_ptr == (*node_ptr)->l) {
            block_t *old_right = (*node_ptr)->r;
            *node_ptr = *pred_ptr; /* Replace target with its left child. */
            (*node_ptr)->r = old_right; /* Attach the original right subtree. */
            assert(*node_ptr != (*node_ptr)->l);
            assert(*node_ptr != (*node_ptr)->r);
        } else {
            /* The predecessor is deeper in the left subtree. */
            block_t *old_left = (*node_ptr)->l;
            block_t *old_right = (*node_ptr)->r;
            block_t *pred_node = *pred_ptr;
            /* Remove the predecessor from its original location. */
            remove_free_tree(&old_left, *pred_ptr);
            /* Replace the target node with the predecessor. */
            *node_ptr = pred_node;
            (*node_ptr)->l = old_left;
            (*node_ptr)->r = old_right;
            assert(*node_ptr != (*node_ptr)->l);
            assert(*node_ptr != (*node_ptr)->r);
        }
    }
    /* If the target node has one child (or none), simply splice it out. */
    else if ((*node_ptr)->l || (*node_ptr)->r) {
        block_t *child = ((*node_ptr)->l) ? (*node_ptr)->l : (*node_ptr)->r;
        *node_ptr = child;
    } else {
        /* No children: remove the node. */
        *node_ptr = NULL;
    }

    /* Clear the removed node's child pointers to avoid dangling references. */
    target->l = NULL;
    target->r = NULL;
}
/* Utility function to create a new node. */
block_t *new_block(size_t size) {
    block_t *node = (block_t *)malloc(sizeof(block_t));
    node->size = size;
    node->l = node->r = NULL;
    return node;
}

/* Inserts a node into the binary search tree. */
void insert_free_tree(block_t **root, block_t *node) {
    if (*root == NULL) {
        *root = node;
        return;
    }

    if (node->size < (*root)->size)
        insert_free_tree(&((*root)->l), node);
    else
        insert_free_tree(&((*root)->r), node);
}



/* Main function to test the tree implementation. */
int main() {
    block_t *root = NULL;
    srand(time(NULL));

    int array_size = 10000;

    block_t **rand_table = (block_t **)malloc(array_size * sizeof(block_t *));
    if (!rand_table) {
        fprintf(stderr, "Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < array_size; i++)
        rand_table[i] = new_block(i);

 //   for (int i = array_size-1; i >=0 ; i--) {
 //       int r = rand() % (i + 1);
 //       block_t *temp = rand_table[r];
 //       rand_table[r] = rand_table[i];
 //       rand_table[i] = temp;
 //   }

    for (int i = 0; i < array_size; i++) {
        insert_free_tree(&root, rand_table[i]);
    }
//    for (int i = array_size-1; i >=0 ; i--) {
//        int r = rand() % (i + 1);
//        block_t *temp = rand_table[r];
//        rand_table[r] = rand_table[i];
//        rand_table[i] = temp;
//    }

    for (int i = array_size-1; i > array_size/2; i--) {
        block_t *remove_b = rand_table[i];
        remove_free_tree(&root, remove_b);
    }

    free(rand_table);
    return 0;
}
