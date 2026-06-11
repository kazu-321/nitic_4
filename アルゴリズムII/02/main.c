#include "rbtree.h"

#include <stdio.h>

static void print_header(const char *title)
{
    printf("\n== %s ==\n", title);
}

static void print_search_result(RBTree *tree, int key)
{
    RBNode *node = rb_search(tree, key);

    if (node == tree->nil) {
        printf("search %d: not found\n", key);
    } else {
        printf("search %d: found (%d,%c)\n", key, node->key,
               node->color == RB_RED ? 'R' : 'B');
    }
}

int main(void)
{
    RBTree *tree = rb_create();
    int insert_seq[] = {7, 4, 3, 1, 5, 2};
    int delete_seq[] = {2, 4, 7, 3, 5, 1};
    size_t i;

    print_header("insert");
    for (i = 0; i < sizeof(insert_seq) / sizeof(insert_seq[0]); ++i) {
        printf("\ninsert %d\n", insert_seq[i]);
        rb_insert(tree, insert_seq[i]);
        rb_print(tree);
    }

    print_header("search");
    print_search_result(tree, 3);
    print_search_result(tree, 6);

    print_header("delete");
    for (i = 0; i < sizeof(delete_seq) / sizeof(delete_seq[0]); ++i) {
        printf("\ndelete %d\n", delete_seq[i]);
        rb_delete(tree, delete_seq[i]);
        rb_print(tree);
    }

    rb_free(tree);
    return 0;
}
