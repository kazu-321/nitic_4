#include "msp.h"

#include <stdio.h>
#include <stdlib.h>

/* input data */
extern unsigned num;
extern int      data[];

int main () {
    printf ("%d\n", max_subarray (0, num));
}

int max_subarray (unsigned from, unsigned num) {
    /* stop recursive call */
    if (num == 1) {
        return data[from];
    }
    if (num == 2) {
        int max = data[from];
        if (data[from + 1] > max) {
            max = data[from + 1];
        }
        if (data[from] + data[from + 1] > max) {
            max = data[from] + data[from + 1];
        }
        return max;
    }
    /* divide */
    unsigned mid        = num / 2;
    unsigned left_num   = mid;
    unsigned left_from  = from;
    unsigned right_num  = num - mid;
    unsigned right_from = from + mid;

    /* conquer */
    int left_max  = max_subarray (left_from, left_num);
    int right_max = max_subarray (right_from, right_num);

    int left_cross_max  = data[left_from + left_num - 1];
    int right_cross_max = data[right_from];
    int sum             = 0;
    for (unsigned i = 0; i < left_num; i++) {
        sum += data[left_from + left_num - 1 - i];
        if (sum > left_cross_max) {
            left_cross_max = sum;
        }
    }
    sum = 0;
    for (unsigned i = 0; i < right_num; i++) {
        sum += data[right_from + i];
        if (sum > right_cross_max) {
            right_cross_max = sum;
        }
    }

    int cross_max = left_cross_max + right_cross_max;
    /* combine */
    int max = left_max;
    if (right_max > max) {
        max = right_max;
    }
    if (cross_max > max) {
        max = cross_max;
    }

    return max;
}
