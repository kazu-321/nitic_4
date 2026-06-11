#!/usr/bin/env bash
set -euo pipefail

gcc -std=c11 -Wall -Wextra -pedantic -o rbtree main.c rbtree.c
./rbtree | tee result.txt
rm ./rbtree
