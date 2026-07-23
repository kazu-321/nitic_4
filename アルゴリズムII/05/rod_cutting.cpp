#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

void solve_rod_cutting (const vector<int> &price, vector<int> &max_value, vector<int> &first_cut) {
    int max_length = static_cast<int> (price.size ()) - 1;
    max_value.assign (max_length + 1, 0);
    first_cut.assign (max_length + 1, 0);

    for (int length = 1; length <= max_length; ++length) {
        int best_value = price[length];
        int best_cut   = length;

        for (int cut = 1; cut < length; ++cut) {
            int candidate = price[cut] + max_value[length - cut];
            if (candidate > best_value) {
                best_value = candidate;
                best_cut   = cut;
            }
        }

        max_value[length] = best_value;
        first_cut[length] = best_cut;
    }
}

vector<int> make_cut_list (int length, const vector<int> &first_cut) {
    vector<int> cut_list;
    int          remaining = length;

    while (remaining > 0) {
        int cut = first_cut[remaining];
        cut_list.push_back (cut);
        remaining -= cut;
    }

    return cut_list;
}

void print_result (const vector<int> &max_value, const vector<int> &first_cut) {
    cout << "┌───────────";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) cout << "┬─────";
    cout << "┐" << endl;

    cout << "│ n         │";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) {
        cout << " " << left << setw (4) << length << "│";
    }
    cout << endl;

    cout << "├───────────";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) cout << "┼─────";
    cout << "┤" << endl;

    cout << "│ max_value │";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) {
        cout << " " << left << setw (4) << max_value[length] << "│";
    }
    cout << endl;

    cout << "├───────────";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) cout << "┼─────";
    cout << "┤" << endl;

    cout << "│ cut       │";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) {
        vector<int>   cut_list = make_cut_list (length, first_cut);
        ostringstream cut_text;
        for (int i = 0; i < static_cast<int> (cut_list.size ()); ++i) {
            if (i > 0) cut_text << "+";
            cut_text << cut_list[i];
        }
        cout << " " << left << setw (4) << cut_text.str () << "│";
    }
    cout << endl;

    cout << "└───────────";
    for (int length = 1; length < static_cast<int> (max_value.size ()); ++length) cout << "┴─────";
    cout << "┘" << endl;
}

int main () {
    vector<int> price = {0, 1, 5, 8, 9, 10, 17, 17, 20, 24, 30};
    vector<int> max_value;
    vector<int> first_cut;

    solve_rod_cutting (price, max_value, first_cut);
    print_result (max_value, first_cut);
    return 0;
}
