rm -f result.txt
g++ ramen.cpp -o ramen
./ramen >> result.txt
rm ramen
