#include <iostream>
using namespace std;

int main() {
    const int PORK_STOCK = 7000;
    const int CHICKEN_STOCK = 8000;
    const int BEEF_STOCK = 3000;

    int bestX = 0;
    int bestY = 0;
    int bestZ = 0;
    int bestSales = 0;

    for (int x = 0; x <= 100; x++) {
        for (int y = 0; y <= 100; y++) {
            for (int z = 0; z <= 100; z++) {
                int pork = 150 * x + 100 * y + 50 * z;
                int chicken = 150 * x + 250 * y + 50 * z;
                int beef = 150 * z;

                if (pork <= PORK_STOCK &&
                    chicken <= CHICKEN_STOCK &&
                    beef <= BEEF_STOCK) {

                    int sales = 850 * x + 950 * y + 1100 * z;

                    if (sales > bestSales) {
                        bestSales = sales;
                        bestX = x;
                        bestY = y;
                        bestZ = z;
                    }
                }
            }
        }
    }

    cout << "豚鶏Wつけ麺: " << bestX << "個" << endl;
    cout << "鶏特盛つけ麺: " << bestY << "個" << endl;
    cout << "デラックスつけ麺: " << bestZ << "個" << endl;
    cout << "最大売上: " << bestSales << "円" << endl;

    return 0;
}