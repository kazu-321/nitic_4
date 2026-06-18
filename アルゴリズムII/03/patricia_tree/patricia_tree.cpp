#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

class Node {
   public:
    string            label;
    bool              is_end;
    map<char, Node *> children;

    Node (const string &l = "", bool end = false) : label (l), is_end (end) {}
};

class PatriciaTree {
   private:
    Node *root;

    // 描画関係
    struct RenderedTree {
        vector<vector<string>> cells;
        int                    width;
        int                    root_x;
    };

    static RenderedTree makeEndMarker () {
        return {vector<vector<string>>{{"#"}}, 1, 0};
    }

    static void trimRight (vector<string> &row) {
        while (!row.empty () && row.back () == " ") row.pop_back ();
    }

    static void putText (vector<vector<string>> &grid, int row, int col, const string &text) {
        for (int i = 0; i < static_cast<int> (text.size ()); ++i) {
            if (row >= 0 && row < static_cast<int> (grid.size ()) && col + i >= 0 && col + i < static_cast<int> (grid[row].size ())) {
                grid[row][col + i] = string (1, text[i]);
            }
        }
    }

    static int cellWidth (const string &text) {
        if (text == "┃" || text == "┏" || text == "┓" || text == "━" || text == "┳") return 1;
        return static_cast<int> (text.size ());
    }

    static void putToken (vector<vector<string>> &grid, int row, int col, const string &token) {
        if (row >= 0 && row < static_cast<int> (grid.size ()) && col >= 0 && col < static_cast<int> (grid[row].size ())) {
            grid[row][col] = token;
        }
    }

    RenderedTree renderTree (Node *node, bool is_root) const {
        string label = is_root ? "ROOT" : node->label;

        if (node->children.empty () && !node->is_end) {
            int                    label_offset = static_cast<int> (label.size () / 2);
            int                    width        = max (label_offset + static_cast<int> (label.size ()), 1);
            vector<vector<string>> grid (2, vector<string> (width, " "));
            int                    label_start = label_offset;
            putText (grid, 0, label_start, label);
            putToken (grid, 1, label_offset, "┃");
            return {grid, width, label_offset};
        }

        vector<RenderedTree> children;
        children.reserve (node->children.size () + (node->is_end ? 1 : 0));
        for (auto &[_, child] : node->children) {
            children.push_back (renderTree (child, false));
        }
        if (node->is_end) {
            children.push_back (makeEndMarker ());
        }

        const int   gap = 2;
        vector<int> child_centers_rel;
        child_centers_rel.reserve (children.size ());
        int children_width = 0;
        for (size_t i = 0; i < children.size (); ++i) {
            if (i > 0) children_width += gap;
            child_centers_rel.push_back (children_width + children[i].root_x);
            children_width += children[i].width;
        }

        int label_offset     = static_cast<int> (label.size () / 2);
        int child_anchor_rel = child_centers_rel.empty () ? 0 : (child_centers_rel.front () + child_centers_rel.back () + 1) / 2;
        int anchor           = max (label_offset, child_anchor_rel);
        int label_start      = anchor - label_offset;
        int children_start   = anchor - child_anchor_rel;
        int width            = max (label_start + static_cast<int> (label.size ()), children_start + children_width);
        int max_child_height = 0;
        for (const auto &child : children) {
            max_child_height = max (max_child_height, static_cast<int> (child.cells.size ()));
        }
        vector<vector<string>> grid (2 + max_child_height, vector<string> (width, " "));

        putText (grid, 0, label_start, label);

        for (size_t i = 0; i < children.size (); ++i) {
            int offset = children_start + (child_centers_rel[i] - children[i].root_x);
            for (int row = 0; row < static_cast<int> (children[i].cells.size ()); ++row) {
                for (int col = 0; col < static_cast<int> (children[i].cells[row].size ()); ++col) {
                    if (children[i].cells[row][col] != " ") {
                        grid[row + 2][offset + col] = children[i].cells[row][col];
                    }
                }
            }
        }

        if (children.size () == 1) {
            putToken (grid, 1, children_start + child_centers_rel[0], "┃");
        } else {
            int         first = children_start + child_centers_rel.front ();
            int         last  = children_start + child_centers_rel.back ();
            vector<int> splits;
            splits.reserve (children.size () - 1);
            for (size_t i = 0; i + 1 < child_centers_rel.size (); ++i) {
                splits.push_back (children_start + (child_centers_rel[i] + child_centers_rel[i + 1] + 1) / 2);
            }
            putToken (grid, 1, first, "┏");
            putToken (grid, 1, last, "┓");
            for (int col = first + 1; col < last; ++col) {
                bool is_split = false;
                for (int split : splits) {
                    if (col == split) {
                        is_split = true;
                        break;
                    }
                }
                if (!is_split) {
                    putToken (grid, 1, col, "━");
                }
            }
            for (int split : splits) {
                putToken (grid, 1, split, "┻");
            }
        }

        for (auto &row : grid) trimRight (row);
        while (!grid.empty () && grid.back ().empty ()) grid.pop_back ();
        return {grid, width, anchor};
    }

    bool erase (Node *node, const string &word, int depth) {
        for (auto it = node->children.begin (); it != node->children.end (); ++it) {
            char          key   = it->first;
            Node         *child = it->second;
            const string &label = child->label;
            int           len   = 0;
            while (depth + len < word.size () && len < label.size () && word[depth + len] == label[len]) {
                ++len;
            }

            if (len == label.size ()) {
                if (depth + len == word.size ()) {
                    if (!child->is_end) return false;
                    child->is_end = false;

                    if (child->children.empty ()) {
                        delete child;
                        node->children.erase (key);
                        return true;
                    }

                    if (child->children.size () == 1 && !child->is_end) {
                        auto  child_it      = child->children.begin ();
                        Node *grandchild    = child_it->second;
                        grandchild->label   = child->label + grandchild->label;
                        node->children[key] = grandchild;
                        delete child;
                    }

                    return true;
                }
                bool result = erase (child, word, depth + len);

                if (result && !child->is_end && child->children.empty ()) {
                    delete child;
                    node->children.erase (key);
                } else if (result && !child->is_end && child->children.size () == 1) {
                    auto  child_it      = child->children.begin ();
                    Node *grandchild    = child_it->second;
                    grandchild->label   = child->label + grandchild->label;
                    node->children[key] = grandchild;
                    delete child;
                }

                return result;
            }
        }
        return false;
    }

   public:
    PatriciaTree () {
        root = new Node ();
    }

    ~PatriciaTree () {
        deleteTree (root);
    }

   private:
    void deleteTree (Node *node) {
        if (node) {
            for (auto &[_, child] : node->children) {
                deleteTree (child);
            }
            delete node;
        }
    }

   public:
    void insert (const string &word) {
        Node *node = root;
        int   i    = 0;
        while (i < word.size ()) {
            char ch = word[i];
            if (!node->children.count (ch)) {
                node->children[ch] = new Node (word.substr (i), true);
                return;
            }

            Node         *child = node->children[ch];
            const string &label = child->label;
            int           j     = 0;
            while (i + j < word.size () && j < label.size () && word[i + j] == label[j]) {
                ++j;
            }

            if (j == label.size ()) {
                node = child;
                i += j;
                continue;
            }

            Node *split                      = new Node (label.substr (0, j), false);
            child->label                     = label.substr (j);
            split->children[child->label[0]] = child;
            node->children[ch]               = split;

            if (i + j < word.size ()) {
                split->children[word[i + j]] = new Node (word.substr (i + j), true);
            } else {
                split->is_end = true;
            }
            return;
        }
        node->is_end = true;
    }

    bool search (const string &word) const {
        Node *node = root;
        int   i    = 0;
        while (i < word.size ()) {
            char ch = word[i];
            if (!node->children.count (ch)) return false;

            Node         *child = node->children.at (ch);
            const string &label = child->label;
            int           j     = 0;
            while (i + j < word.size () && j < label.size () && word[i + j] == label[j]) {
                ++j;
            }
            if (j < label.size ()) return false;
            node = child;
            i += j;
        }
        return node->is_end;
    }

    bool startsWith (const string &prefix) const {
        Node *node = root;
        int   i    = 0;
        while (i < prefix.size ()) {
            char ch = prefix[i];
            if (!node->children.count (ch)) return false;

            Node         *child = node->children.at (ch);
            const string &label = child->label;
            int           j     = 0;
            while (i + j < prefix.size () && j < label.size () && prefix[i + j] == label[j]) {
                ++j;
            }
            if (j < label.size () && i + j < prefix.size ()) return false;
            node = child;
            i += j;
        }
        return true;
    }

    bool erase (const string &word) {
        return erase (root, word, 0);
    }

    void print () const {
        auto rendered = renderTree (root, true);
        for (const auto &row : rendered.cells) {
            string line;
            for (const auto &cell : row) line += cell;
            cout << line << endl;
        }
    }
};

int main () {
    PatriciaTree   tree;
    vector<string> insert_words = {"man", "mane", "dry", "dried", "mat"};
    vector<string> search_words = {"mane", "many", "drie"};
    vector<string> delete_words = {"dried", "mat", "dry", "man", "mane"};

    for (const auto &w : insert_words) {
        cout << "Inserting: " << w << endl;
        tree.insert (w);

        cout << endl;
        tree.print ();
        cout << "--------------------------------------------------" << endl;
    }

    cout << "\nSearch Results:" << endl;
    for (const auto &w : search_words) {
        cout << w << ": " << tree.search (w) << " (startsWith: " << tree.startsWith (w) << ")\n";
    }
    cout << "--------------------------------------------------" << endl;

    for (const auto &w : delete_words) {
        cout << "Deleting: " << w << endl;
        tree.erase (w);

        cout << endl;
        tree.print ();
        cout << "--------------------------------------------------" << endl;
    }
    return 0;
}
