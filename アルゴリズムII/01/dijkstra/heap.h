# include <stdlib.h>
# include <stdio.h>

typedef struct {
	unsigned num;
	unsigned *val;
} HEAP;

HEAP *new_heap(unsigned num);
void free_heap(HEAP *heap);

unsigned remove_root(HEAP *heap);
void change_heap(HEAP *heap, unsigned pos);
