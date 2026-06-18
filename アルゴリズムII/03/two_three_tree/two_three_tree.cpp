#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;

struct Node {
    vector<int>   keys;
    vector<Node*> children;
    Node*         parent = nullptr;

    bool is_leaf () const {
        return children.empty ();
    }
};

class TwoThreeTree {
  private:
    Node*      root = nullptr;
    vector<int> values;

    static int min_keys_for_height (int height) {
        int keys = 1;
        for (int i = 0; i < height; ++i) {
            keys = keys * 2 + 1;
        }
        return keys;
    }

    static int max_keys_for_height (int height) {
        int keys = 2;
        for (int i = 0; i < height; ++i) {
            keys = keys * 3 + 2;
        }
        return keys;
    }

    static void destroy (Node* node) {
        if (!node) return;
        for (Node* child : node->children) destroy (child);
        delete node;
    }

    static bool partition_count (int total, int parts, int min_part, int max_part, vector<int>& out) {
        if (total < parts * min_part || total > parts * max_part) return false;

        out.assign (parts, min_part);
        int extra = total - parts * min_part;
        int cap   = max_part - min_part;

        for (int i = 0; i < parts; ++i) {
            int remaining_parts = parts - i;
            int low             = max (0, extra - (remaining_parts - 1) * cap);
            int high            = min (cap, extra);
            int target          = extra / remaining_parts;
            int take            = min (max (target, low), high);
            out[i] += take;
            extra -= take;
        }

        return extra == 0;
    }

    Node* build_subtree (const vector<int>& arr, int l, int r, int height) {
        int n = r - l;
        if (n == 0) return nullptr;

        Node* node = new Node ();
        if (height == 0) {
            node->keys.insert (node->keys.end (), arr.begin () + l, arr.begin () + r);
            return node;
        }

        int min_child = min_keys_for_height (height - 1);
        int max_child = max_keys_for_height (height - 1);

        struct Option {
            int        key_count = 0;
            vector<int> child_sizes;
            int        score = 0;
        };

        Option best;
        bool   has_best = false;

        auto consider = [&] (int key_count, int child_count) {
            vector<int> child_sizes;
            if (!partition_count (n - key_count, child_count, min_child, max_child, child_sizes)) return;

            int mn = *min_element (child_sizes.begin (), child_sizes.end ());
            int mx = *max_element (child_sizes.begin (), child_sizes.end ());
            Option opt {key_count, child_sizes, mx - mn};

            if (!has_best || opt.score < best.score || (opt.score == best.score && opt.key_count > best.key_count)) {
                best     = opt;
                has_best = true;
            }
        };

        consider (1, 2);
        consider (2, 3);

        if (!has_best) {
            destroy (node);
            throw logic_error ("failed to build a valid 2-3 tree");
        }

        if (best.key_count == 1) {
            int left_count = best.child_sizes[0];
            int sep        = l + left_count;
            node->keys.push_back (arr[sep]);

            Node* left  = build_subtree (arr, l, sep, height - 1);
            Node* right = build_subtree (arr, sep + 1, r, height - 1);
            node->children = {left, right};
            left->parent   = node;
            right->parent  = node;
        } else {
            int c1 = best.child_sizes[0];
            int c2 = best.child_sizes[1];

            int sep1 = l + c1;
            int sep2 = sep1 + 1 + c2;
            node->keys.push_back (arr[sep1]);
            node->keys.push_back (arr[sep2]);

            Node* left   = build_subtree (arr, l, sep1, height - 1);
            Node* middle = build_subtree (arr, sep1 + 1, sep2, height - 1);
            Node* right  = build_subtree (arr, sep2 + 1, r, height - 1);
            node->children = {left, middle, right};
            left->parent    = node;
            middle->parent  = node;
            right->parent   = node;
        }

        return node;
    }

    static bool search_node (Node* node, int key) {
        if (!node) return false;

        for (int k : node->keys) {
            if (key == k) return true;
        }

        if (node->is_leaf ()) return false;

        if (node->keys.size () == 1) {
            if (key < node->keys[0]) return search_node (node->children[0], key);
            return search_node (node->children[1], key);
        }

        if (key < node->keys[0]) return search_node (node->children[0], key);
        if (key < node->keys[1]) return search_node (node->children[1], key);
        return search_node (node->children[2], key);
    }

    void rebuild () {
        destroy (root);
        root = nullptr;

        if (values.empty ()) return;

        int height = 0;
        while (max_keys_for_height (height) < static_cast<int> (values.size ())) ++height;
        root = build_subtree (values, 0, static_cast<int> (values.size ()), height);
        if (root) root->parent = nullptr;
    }


    struct RenderBox {
        vector<string> lines;
        size_t         width = 0;
    };

    static string repeat_char (char c, size_t n) {
        return string (n, c);
    }

    RenderBox render_box (Node* node) const {
        RenderBox box;
        if (!node) return box;

        string label = "[";
        for (size_t i = 0; i < node->keys.size (); ++i) {
            label += to_string (node->keys[i]);
            if (i + 1 < node->keys.size ()) label += ", ";
        }
        label += "]";

        if (node->children.empty ()) {
            box.lines.push_back (label);
            box.width = label.size ();
            return box;
        }

        vector<RenderBox> child_boxes;
        child_boxes.reserve (node->children.size ());
        for (Node* child : node->children) child_boxes.push_back (render_box (child));

        size_t child_gap = 3;
        size_t children_width = 0;
        size_t child_height = 0;
        for (size_t i = 0; i < child_boxes.size (); ++i) {
            children_width += child_boxes[i].width;
            if (i + 1 < child_boxes.size ()) children_width += child_gap;
            child_height = max (child_height, child_boxes[i].lines.size ());
        }

        box.width = max (label.size (), children_width);
        size_t label_left = (box.width - label.size ()) / 2;
        box.lines.push_back (repeat_char (' ', label_left) + label + repeat_char (' ', box.width - label_left - label.size ()));

        string connector (box.width, ' ');
        size_t start_col = (box.width - children_width) / 2;
        size_t cursor = start_col;
        if (child_boxes.size () == 2) {
            size_t left_pos  = cursor + child_boxes[0].width / 2;
            cursor += child_boxes[0].width + child_gap;
            size_t right_pos = cursor + child_boxes[1].width / 2;
            if (left_pos < connector.size ()) connector[left_pos] = '/';
            if (right_pos < connector.size ()) connector[right_pos] = '\\';
        } else if (child_boxes.size () == 3) {
            size_t left_pos   = cursor + child_boxes[0].width / 2;
            cursor += child_boxes[0].width + child_gap;
            size_t middle_pos = cursor + child_boxes[1].width / 2;
            cursor += child_boxes[1].width + child_gap;
            size_t right_pos  = cursor + child_boxes[2].width / 2;
            if (left_pos < connector.size ()) connector[left_pos] = '/';
            if (middle_pos < connector.size ()) connector[middle_pos] = '|';
            if (right_pos < connector.size ()) connector[right_pos] = '\\';
        }
        box.lines.push_back (connector);

        vector<string> merged (child_height, string (box.width, ' '));
        cursor = start_col;
        for (size_t child_index = 0; child_index < child_boxes.size (); ++child_index) {
            const RenderBox& child = child_boxes[child_index];
            size_t child_left = cursor;
            for (size_t line = 0; line < child.lines.size (); ++line) {
                string padded = child.lines[line] + repeat_char (' ', child.width - child.lines[line].size ());
                merged[line].replace (child_left, child.width, padded);
            }
            cursor += child.width + child_gap;
        }

        for (const string& line : merged) box.lines.push_back (line);
        return box;
    }

    void print_rendered (const RenderBox& box) const {
        for (const string& line : box.lines) {
            string trimmed = line;
            while (!trimmed.empty () && trimmed.back () == ' ') trimmed.pop_back ();
            cout << trimmed << endl;
        }
    }

  public:
    ~TwoThreeTree () {
        destroy (root);
    }

    void insert (int key) {
        auto it = lower_bound (values.begin (), values.end (), key);
        if (it != values.end () && *it == key) return;
        values.insert (it, key);
        rebuild ();
    }

    bool search (int key) const {
        return search_node (root, key);
    }

    void print () const {
        if (!root) {
            cout << "[]" << endl;
            return;
        }
        print_rendered (render_box (root));
    }

    void remove (int key) {
        auto it = lower_bound (values.begin (), values.end (), key);
        if (it == values.end () || *it != key) return;
        values.erase (it);
        rebuild ();
    }
};

int main () {
    vector<int> odds;
    for (int i = 1; i <= 21; i += 2) odds.push_back (i);

    random_device rd;
    mt19937       g (rd ());

    vector<int> insert_order = odds;
    shuffle (insert_order.begin (), insert_order.end (), g);

    TwoThreeTree tree;
    for (int v : insert_order) {
        cout << "Insert " << v << endl;
        tree.insert (v);

        cout << endl;
        tree.print ();
        cout << "--------------------------------------------------" << endl;
    }

    cout << "Search 9: " << (tree.search (9) ? "Found" : "Not found") << endl;
    cout << "Search 18: " << (tree.search (18) ? "Found" : "Not found") << endl;

    vector<int> erase_order = odds;
    do {
        shuffle (erase_order.begin (), erase_order.end (), g);
    } while (erase_order == insert_order);

    for (int v : erase_order) {
        cout << "Remove " << v << endl;
        tree.remove (v);

        cout << endl;
        tree.print ();
        cout << "--------------------------------------------------" << endl;
    }

    return 0;
}
