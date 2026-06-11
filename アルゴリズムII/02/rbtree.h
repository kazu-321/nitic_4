#ifndef RBTREE_H
#define RBTREE_H

#include <stdio.h>

typedef enum {
    RB_BLACK = 0,
    RB_RED = 1
} RBColor;

typedef struct RBNode {
    int key;
    RBColor color;
    struct RBNode *left;
    struct RBNode *right;
    struct RBNode *parent;
} RBNode;

typedef struct RBTree {
    RBNode *root;
    RBNode *nil;
} RBTree;

RBTree *rb_create(void);
void    rb_free(RBTree *tree);

RBNode *rb_search(const RBTree *tree, int key);
void    rb_insert(RBTree *tree, int key);
void    rb_delete(RBTree *tree, int key);

void    rb_print(const RBTree *tree);

#endif
