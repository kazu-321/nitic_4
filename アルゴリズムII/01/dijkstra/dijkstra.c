# include <limits.h>
# include <stdio.h>
# include "dijkstra.h"
# include "heap.h"
unsigned dist[NMAX];
int main() {
	/* initialise dist */
	dist[0] = 0;
	for ( unsigned  i = 1; i < num_n; i ++ ) dist[i] = UINT_MAX;
	/* initialise heap */
	HEAP *heap = new_heap(num_n);
	for ( unsigned i = 0; i < num_n; i ++ ) heap->val[i] = i;
	/* greedy method */
	while ( heap->num != 0 ) {
		/* get current nearest node */
		unsigned xm = remove_root(heap);
		/* loop for further nodes */
		for ( unsigned i = 0; i < heap->num; i ++ ) {
			unsigned xi = heap->val[i];
			/* check connectivity */
			if ( weight[xm][xi] == 0 ) continue;
			/* di = min{di, dm+w(xm,xi)} */
			int sum = dist[xm] + weight[xm][xi];
			if ( sum <= dist[xi] ) {
				dist[xi] = sum;
				change_heap(heap, i);
			}
		}
	}
	/* output results */
	for ( unsigned xi = 0; xi < num_n; xi ++ ) {
		printf("x[%d] %d\n", xi, dist[xi]);
	}
}
