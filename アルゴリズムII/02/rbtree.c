#include "rbtree.h"

#include <stdlib.h>

static RBNode *rb_create_node(RBTree *tree, int key)
{
    RBNode *node = (RBNode *)malloc(sizeof(RBNode));
    if (node == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->key = key;
    node->color = RB_RED;
    node->left = tree->nil;
    node->right = tree->nil;
    node->parent = tree->nil;
    return node;
}

static void rb_left_rotate(RBTree *tree, RBNode *x)
{
    RBNode *y = x->right;
    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void rb_right_rotate(RBTree *tree, RBNode *y)
{
    RBNode *x = y->left;
    y->left = x->right;
    if (x->right != tree->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

static void rb_insert_fixup(RBTree *tree, RBNode *z)
{
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode *y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rb_left_rotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_right_rotate(tree, z->parent->parent);
            }
        } else {
            RBNode *y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rb_right_rotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                rb_left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

static void rb_transplant(RBTree *tree, RBNode *u, RBNode *v)
{
    if (u->parent == tree->nil) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

static RBNode *rb_minimum(const RBTree *tree, RBNode *node)
{
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

static void rb_delete_fixup(RBTree *tree, RBNode *x)
{
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNode *w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_right_rotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                rb_left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            RBNode *w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                rb_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    rb_left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                rb_right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void rb_free_nodes(RBTree *tree, RBNode *node)
{
    if (node == tree->nil) {
        return;
    }
    rb_free_nodes(tree, node->left);
    rb_free_nodes(tree, node->right);
    free(node);
}

static void rb_print_rec(const RBTree *tree, const RBNode *node, int depth)
{
    int i;

    if (node == tree->nil) {
        return;
    }

    rb_print_rec(tree, node->right, depth + 1);
    for (i = 0; i < depth; ++i) {
        printf("    ");
    }
    printf("%d(%c)\n", node->key, node->color == RB_RED ? 'R' : 'B');
    rb_print_rec(tree, node->left, depth + 1);
}

RBTree *rb_create(void)
{
    RBTree *tree = (RBTree *)malloc(sizeof(RBTree));
    if (tree == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    tree->nil = (RBNode *)malloc(sizeof(RBNode));
    if (tree->nil == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    tree->nil->key = 0;
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    return tree;
}

void rb_free(RBTree *tree)
{
    if (tree == NULL) {
        return;
    }
    rb_free_nodes(tree, tree->root);
    free(tree->nil);
    free(tree);
}

RBNode *rb_search(const RBTree *tree, int key)
{
    RBNode *node = tree->root;

    while (node != tree->nil && key != node->key) {
        if (key < node->key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return node;
}

void rb_insert(RBTree *tree, int key)
{
    RBNode *z = rb_create_node(tree, key);
    RBNode *y = tree->nil;
    RBNode *x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == tree->nil) {
        tree->root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    rb_insert_fixup(tree, z);
}

void rb_delete(RBTree *tree, int key)
{
    RBNode *z = rb_search(tree, key);
    RBNode *y = z;
    RBNode *x;
    RBColor y_original_color = y->color;

    if (z == tree->nil) {
        return;
    }

    if (z->left == tree->nil) {
        x = z->right;
        rb_transplant(tree, z, z->right);
    } else if (z->right == tree->nil) {
        x = z->left;
        rb_transplant(tree, z, z->left);
    } else {
        y = rb_minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            rb_transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        rb_transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    if (y_original_color == RB_BLACK) {
        rb_delete_fixup(tree, x);
    }
}

void rb_print(const RBTree *tree)
{
    if (tree->root == tree->nil) {
        printf("(empty)\n");
        return;
    }

    rb_print_rec(tree, tree->root, 0);
}
