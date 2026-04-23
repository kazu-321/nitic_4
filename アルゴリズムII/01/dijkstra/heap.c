# include <stdlib.h>
# include <stdio.h>
# include "heap.h"
# include "dijkstra.h"
extern unsigned dist[NMAX];

void change_heap(HEAP *heap, unsigned pos) {
	/* make it */
	if (pos == 0 || pos >= heap->num) {
		return;
	}

	unsigned parent = (pos - 1) / 2;

	if (dist[heap->val[pos]] < dist[heap->val[parent]]) {
		unsigned tmp = heap->val[pos];
		heap->val[pos] = heap->val[parent];
		heap->val[parent] = tmp;
		change_heap(heap, parent);
	}
}

static void heapify(HEAP *heap, unsigned pos) {
	/* make it */
	unsigned left = pos * 2 + 1;
	unsigned right = pos * 2 + 2;
	unsigned smallest = pos;

	if (left < heap->num && dist[heap->val[left]] < dist[heap->val[smallest]]) {
		smallest = left;
	}
	if (right < heap->num && dist[heap->val[right]] < dist[heap->val[smallest]]) {
		smallest = right;
	}
	if (smallest != pos) {
		unsigned tmp = heap->val[pos];
		heap->val[pos] = heap->val[smallest];
		heap->val[smallest] = tmp;
		heapify(heap, smallest);
	}
}

unsigned remove_root(HEAP *heap) {
	unsigned root = heap->val[0];
	heap->val[0] = heap->val[-- heap->num];
	heapify(heap, 0);
	return root;
}

HEAP *new_heap(unsigned num) {
	HEAP *new;
	if ( (new = (HEAP *)malloc(sizeof(HEAP))) == NULL ) {
		perror("no more memory");
		exit(EXIT_FAILURE);
	}
	new->num = num;
	if ( (new->val = (unsigned *)calloc(num, sizeof(unsigned))) == NULL ) {
		perror("too large");
	}
	return new;
}

void free_heap(HEAP *heap) {
	free(heap->val);
	free(heap);
}

