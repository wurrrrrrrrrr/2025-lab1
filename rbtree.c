#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef enum { RED, BLACK } color_t;

typedef struct block {
    size_t size;
    struct block *l, *r, *parent;
    color_t color;
} block_t;

block_t *root = NULL;

// Function prototypes
void rotate_left(block_t **root, block_t *x);
void rotate_right(block_t **root, block_t *x);
void insert_fixup(block_t **root, block_t *z);
void delete_fixup(block_t **root, block_t *x);
void rb_insert(block_t **root, block_t *z);
block_t *rb_minimum(block_t *node);
void rb_transplant(block_t **root, block_t *u, block_t *v);
void rb_delete(block_t **root, block_t *z);
block_t *new_block(size_t size);
block_t *find_block(block_t *root, size_t size);

// Left rotate
void rotate_left(block_t **root, block_t *x) {
    block_t *y = x->r;
    x->r = y->l;
    if (y->l) y->l->parent = x;
    y->parent = x->parent;
    if (!x->parent) *root = y;
    else if (x == x->parent->l) x->parent->l = y;
    else x->parent->r = y;
    y->l = x;
    x->parent = y;
}

// Right rotate
void rotate_right(block_t **root, block_t *y) {
    block_t *x = y->l;
    y->l = x->r;
    if (x->r) x->r->parent = y;
    x->parent = y->parent;
    if (!y->parent) *root = x;
    else if (y == y->parent->l) y->parent->l = x;
    else y->parent->r = x;
    x->r = y;
    y->parent = x;
}

// Fix RB Tree after insertion
void insert_fixup(block_t **root, block_t *z) {
    while (z->parent && z->parent->color == RED) {
        if (z->parent == z->parent->parent->l) {
            block_t *y = z->parent->parent->r;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->r) {
                    z = z->parent;
                    rotate_left(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotate_right(root, z->parent->parent);
            }
        } else {
            block_t *y = z->parent->parent->l;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->l) {
                    z = z->parent;
                    rotate_right(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotate_left(root, z->parent->parent);
            }
        }
    }
    (*root)->color = BLACK;
}

// Insert a block into RB tree
void rb_insert(block_t **root, block_t *z) {
    block_t *y = NULL;
    block_t *x = *root;

    while (x) {
        y = x;
        if (z->size < x->size)
            x = x->l;
        else
            x = x->r;
    }
    z->parent = y;

    if (!y) 
        *root = z;
    else if (z->size < y->size) 
        y->l = z;
    else 
        y->r = z;

    z->l = NULL;
    z->r = NULL;
    z->color = RED;
    insert_fixup(root, z);
}

// Find the smallest (leftmost) node in a subtree.
block_t *rb_minimum(block_t *node) {
    while (node->l) {  // Keep going left
        node = node->l;
    }
    return node;  // Leftmost node is the minimum
}

// Delete a block from RB tree
void rb_delete(block_t **root, block_t *z) {
    block_t *y = z, *x;
    color_t y_original_color = y->color;

    if (!z->l) {
        x = z->r;
        rb_transplant(root, z, z->r);
    } else if (!z->r) {
        x = z->l;
        rb_transplant(root, z, z->l);
    } else {
        y = rb_minimum(z->r);
        y_original_color = y->color;
        x = y->r;
        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            rb_transplant(root, y, y->r);
            y->r = z->r;
            y->r->parent = y;
        }
        rb_transplant(root, z, y);
        y->l = z->l;
        y->l->parent = y;
        y->color = z->color;
    }
    if (y_original_color == BLACK) {
        if (x) {
            delete_fixup(root, x);
        } else {
            //printf("Warning: x is NULL, skipping delete_fixup()\n");
            ;
        }
    }
}

void rb_transplant(block_t **root, block_t *u, block_t *v) {
    if (!u->parent) {  // If u is the root
        *root = v;
    } else if (u == u->parent->l) {  // If u is a left child
        u->parent->l = v;
    } else {  // If u is a right child
        u->parent->r = v;
    }
    
    if (v) {  // Update parent pointer of v (if v is not NULL)
        v->parent = u->parent;
    }
}

void delete_fixup(block_t **root, block_t *x) {
    while (x != *root && (!x || x->color == BLACK) && (x->parent)) {
        if (x == x->parent->l) { // x is a left child
            block_t *w = x->parent->r; // Sibling of x
            
            // Case 1: Sibling w is RED
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotate_left(root, x->parent);
                w = x->parent->r;
            }
	    
            // Case 2: Both of w's children are BLACK
            if ((!w->l || w->l->color == BLACK) && (!w->r || w->r->color == BLACK)) {
                w->color = RED;
                x = x->parent; // Move problem up to parent
            } else {
                // Case 3: w’s left child is RED, right child is BLACK
                if (!w->r || w->r->color == BLACK) {
                    if (w->l) w->l->color = BLACK;
                    w->color = RED;
                    rotate_right(root, w);
                    w = x->parent->r;
                }

                // Case 4: w’s right child is RED
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->r) w->r->color = BLACK;
                rotate_left(root, x->parent);
                x = *root; // End loop
            }
        } else { // Mirror cases where x is a right child
            block_t *w = x->parent->l;    
                
            // Case 1: Sibling w is RED
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rotate_right(root, x->parent);
                w = x->parent->l;
            }

            // Case 2: Both of w's children are BLACK
            if ((!w->l || w->l->color == BLACK) && (!w->r || w->r->color == BLACK)) {
                w->color = RED;
                x = x->parent;
            } else {
                // Case 3: w’s right child is RED, left child is BLACK
                if (!w->l || w->l->color == BLACK) {
                    if (w->r) w->r->color = BLACK;
                    w->color = RED;
                    rotate_left(root, w);
                    w = x->parent->l;
                }

                // Case 4: w’s left child is RED
                w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w->l) w->l->color = BLACK;
                rotate_right(root, x->parent);
                x = *root; // End loop
            }
        }
    }
    if (x) 
        x->color = BLACK; // Restore black balance
}

// Create new block
block_t *new_block(size_t size) {
    block_t *new = (block_t *)malloc(sizeof(block_t)); // FIX: Allocate correct size
    if (!new) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    new->size = size;
    new->color = RED;
    new->l = NULL;
    new->r = NULL;
    new->parent = NULL;
    return new;
}

void print_tree_val(block_t *root) {
    if (!root)
        return;
    if (root->r){
        printf("   %ld->", root->size);
        printf("%ld\n", root->r->size);
        print_tree_val(root->r);
    }
    if (root->l){
        printf("   %ld->", root->size);
        printf("%ld\n", root->l->size);
        print_tree_val(root->l);
    }
}

void print_tree_color(block_t *root, color_t color) {
    if (!root)
        return;
    if (root-> color == color)
        printf("    %ld\n", root->size);
    if (root->r)
        print_tree_color(root->r, color);
    if (root->l)
        print_tree_color(root->l, color);
}

block_t *find_block(block_t *root, size_t size) {
    if (!root)
        return NULL;
    if (root->size == size)
        return root;
    if (root->size > size)
        return find_block(root->l, size);
    else
        return find_block(root->r, size); 
}

void generate_graviz(block_t *root){
    printf("digraph G {\n");
    printf("  subgraph red {\n");
    printf("    node [color=\"red\", style=\"filled\", group=\"red\"]\n");
    print_tree_color(root, RED);
    printf("  }\n");
    
    printf("  subgraph black {\n");
    printf("    node [color=\"black\", style=\"filled\", group=\"black\", fontcolor=\"white\"]\n");
    print_tree_color(root, BLACK);
    printf("  }\n");
    
    print_tree_val(root);
    printf("}\n\n\n\n");
}

int main() {
    srand( time(NULL) );
    int array_size = 10000;
    // Generate random number table
    int rand_table[array_size];
    for (int i = 0; i < array_size; i++)
        rand_table[i] = i;
    // Shuffle the array
    for (int i = array_size-1; i >=0 ; i--) {
        int idx = rand() % (i+1);
        int temp = rand_table[idx];
        rand_table[idx] = rand_table[i];
        rand_table[i] = temp;
    }
    
    for(int i = 0; i < array_size; i++) {
        block_t *new = new_block(rand_table[i]);
        rb_insert(&root, new);
    }
    
    //generate_graviz(root);
    
    // Shuffle the array again
    for (int i = array_size-1; i >=0 ; i--) {
        int idx = rand() % (i+1);
        int temp = rand_table[idx];
        rand_table[idx] = rand_table[i];
        rand_table[i] = temp;
    }

    for(int i = array_size-1; i > array_size/2; i--) {
        //printf("remove: %d\n", rand_table[i]);
        block_t *target = find_block(root, rand_table[i]);
        rb_delete(&root, target);
    }
    // Print the tree after remove nodes
    //generate_graviz(root);
    return 0;
}
