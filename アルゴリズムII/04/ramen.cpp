#include <bits/stdc++.h>
using namespace std;

const double eps         = 1e-9;
const double integer_eps = 1e-7;
const double inf         = 1e100;

int m_count;
int n_count;

vector<int>            basis;
vector<int>            non_basis;
vector<vector<double>> table;

vector<double>         c;
vector<vector<double>> original_a;
vector<double>         original_b;

bool              found_answer = false;
double            best_z       = -inf;
vector<long long> best_x;
long long         node_count = 0;

void pivot_table (int row, int col) {
    double inv = 1.0 / table[row][col];
    for (int i = 0; i < m_count + 2; i++) {
        if (i == row) continue;
        for (int j = 0; j < n_count + 2; j++) {
            if (j == col) continue;
            table[i][j] -= table[row][j] * table[i][col] * inv;
        }
    }
    for (int j = 0; j < n_count + 2; j++) {
        if (j != col) {
            table[row][j] *= inv;
        }
    }
    for (int i = 0; i < m_count + 2; i++) {
        if (i != row) {
            table[i][col] *= -inv;
        }
    }
    table[row][col] = inv;
    swap (basis[row], non_basis[col]);
}

bool simplex_phase (int phase) {
    int obj_row = (phase == 1 ? m_count + 1 : m_count);
    while (true) {
        int col = -1;
        for (int j = 0; j <= n_count; j++) {
            if (phase == 2 && non_basis[j] == -1) {
                continue;
            }
            if (col == -1 || table[obj_row][j] < table[obj_row][col] - eps || (fabs (table[obj_row][j] - table[obj_row][col]) <= eps && non_basis[j] < non_basis[col])) {
                col = j;
            }
        }
        if (table[obj_row][col] >= -eps) {
            return true;
        }
        int row = -1;
        for (int i = 0; i < m_count; i++) {
            if (table[i][col] <= eps) {
                continue;
            }
            if (row == -1) {
                row = i;
                continue;
            }
            double now  = table[i][n_count + 1] / table[i][col];
            double best = table[row][n_count + 1] / table[row][col];

            if (now < best - eps || (fabs (now - best) <= eps && basis[i] < basis[row])) {
                row = i;
            }
        }
        if (row == -1) {
            return false;
        }
        pivot_table (row, col);
    }
}

double solve_simplex (const vector<vector<double>> &a, const vector<double> &b, vector<double> &x, bool branch_mode) {
    m_count = (int)b.size ();
    n_count = (int)c.size ();

    basis.assign (m_count, 0);
    non_basis.assign (n_count + 1, 0);
    table.assign (m_count + 2, vector<double> (n_count + 2, 0.0));
    for (int i = 0; i < m_count; i++) {
        for (int j = 0; j < n_count; j++) {
            table[i][j] = a[i][j];
        }
    }
    for (int i = 0; i < m_count; i++) {
        basis[i]              = n_count + i;
        table[i][n_count]     = -1.0;
        table[i][n_count + 1] = b[i];
    }
    for (int j = 0; j < n_count; j++) {
        non_basis[j]      = j;
        table[m_count][j] = -c[j];
    }
    non_basis[n_count]          = -1;
    table[m_count + 1][n_count] = 1.0;
    int row                     = 0;
    for (int i = 1; i < m_count; i++) {
        if (table[i][n_count + 1] < table[row][n_count + 1]) {
            row = i;
        }
    }
    if (m_count > 0 && table[row][n_count + 1] < -eps) {
        pivot_table (row, n_count);
        if (!simplex_phase (1) || table[m_count + 1][n_count + 1] < -eps) {
            if (branch_mode) {
                return -inf;
            }
            cerr << "infeasible" << endl;
            exit (1);
        }
        for (int i = 0; i < m_count; i++) {
            if (basis[i] == -1) {
                int col = -1;
                for (int j = 0; j <= n_count; j++) {
                    if (col == -1 || table[i][j] < table[i][col] - eps || (fabs (table[i][j] - table[i][col]) <= eps && non_basis[j] < non_basis[col])) {
                        col = j;
                    }
                }
                pivot_table (i, col);
            }
        }
    }
    if (!simplex_phase (2)) {
        cerr << "unbounded" << endl;
        exit (1);
    }
    x.assign (n_count, 0.0);
    for (int i = 0; i < m_count; i++) {
        if (0 <= basis[i] && basis[i] < n_count) {
            x[basis[i]] = table[i][n_count + 1];
        }
    }
    return table[m_count][n_count + 1];
}

double calc_z (const vector<long long> &x) {
    double z = 0.0;
    for (int i = 0; i < (int)c.size (); i++) {
        z += c[i] * x[i];
    }
    return z;
}

bool check_original_constraints (const vector<long long> &x) {
    for (int i = 0; i < (int)original_a.size (); i++) {
        double lhs = 0.0;
        for (int j = 0; j < (int)c.size (); j++) {
            lhs += original_a[i][j] * x[j];
        }
        if (lhs > original_b[i] + integer_eps) {
            return false;
        }
    }
    return true;
}

void branch_and_bound (vector<vector<double>> a, vector<double> b) {
    node_count++;
    vector<double> lp_x;
    double         upper_bound = solve_simplex (a, b, lp_x, true);
    if (upper_bound <= -inf / 2.0) {
        return;
    }
    if (found_answer && upper_bound <= best_z + integer_eps) {
        return;
    }
    int branch_var = -1;
    for (int i = 0; i < (int)c.size (); i++) {
        if (fabs (lp_x[i] - round (lp_x[i])) > integer_eps) {
            branch_var = i;
            break;
        }
    }
    if (branch_var == -1) {
        vector<long long> int_x (c.size ());
        for (int i = 0; i < (int)c.size (); i++) {
            int_x[i] = llround (lp_x[i]);
        }
        if (!check_original_constraints (int_x)) {
            return;
        }
        double z = calc_z (int_x);
        if (!found_answer || z > best_z + integer_eps) {
            found_answer = true;
            best_z       = z;
            best_x       = int_x;
        }
        return;
    }
    double         v    = lp_x[branch_var];
    double         low  = floor (v);
    double         high = ceil (v);
    vector<double> row1 (c.size (), 0.0);
    row1[branch_var] = 1.0;
    a.push_back (row1);
    b.push_back (low);
    branch_and_bound (a, b);
    a.pop_back ();
    b.pop_back ();
    vector<double> row2 (c.size (), 0.0);
    row2[branch_var] = -1.0;
    a.push_back (row2);
    b.push_back (-high);
    branch_and_bound (a, b);
}

int main () {
    c          = {850.0, 950.0, 1100.0};
    original_a = {
        {150.0, 100.0,  50.0},
        {150.0, 250.0,  50.0},
        {  0.0,   0.0, 150.0}
    };
    original_b = {7000.0, 8000.0, 3000.0};

    cout << fixed << setprecision (10);
    vector<double> lp_x;
    double         lp_z = solve_simplex (original_a, original_b, lp_x, false);
    cout << "LP relaxation" << endl;
    for (int i = 0; i < (int)lp_x.size (); i++) {
        cout << "x" << i + 1 << " = " << lp_x[i] << endl;
    }
    cout << "z = " << lp_z << endl;
    best_x.assign (c.size (), 0);
    branch_and_bound (original_a, original_b);
    if (!found_answer) {
        cerr << "integer infeasible" << endl;
        exit (1);
    }
    cout << endl;
    cout << "integer solution" << endl;
    for (int i = 0; i < (int)best_x.size (); i++) {
        cout << "x" << i + 1 << " = " << best_x[i] << endl;
    }
    cout << "z = " << (long long)llround (best_z) << endl;
    return 0;
}