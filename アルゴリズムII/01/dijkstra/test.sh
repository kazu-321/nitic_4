rm -f data/result.txt
echo "===Data 0===" >> data/result.txt
gcc dijkstra.c heap.c data/data_0.c -o dijkstra && ./dijkstra >> data/result.txt
echo "===Data 1===" >> data/result.txt
gcc dijkstra.c heap.c data/data_1.c -o dijkstra && ./dijkstra >> data/result.txt
echo "===Data 2===" >> data/result.txt
gcc dijkstra.c heap.c data/data_2.c -o dijkstra && ./dijkstra >> data/result.txt
echo "===Data 3===" >> data/result.txt
gcc dijkstra.c heap.c data/data_3.c -o dijkstra && ./dijkstra >> data/result.txt
rm dijkstra