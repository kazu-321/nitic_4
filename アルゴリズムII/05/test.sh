rm -f result.txt
g++ rod_cutting.cpp -o rod_cutting
./rod_cutting >> result.txt
rm rod_cutting
