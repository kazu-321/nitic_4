rm -f data/result.txt
gcc msp.c data/data_0.c -o msp && ./msp >> data/result.txt
gcc msp.c data/data_1.c -o msp && ./msp >> data/result.txt
gcc msp.c data/data_2.c -o msp && ./msp >> data/result.txt
gcc msp.c data/data_3.c -o msp && ./msp >> data/result.txt
rm msp