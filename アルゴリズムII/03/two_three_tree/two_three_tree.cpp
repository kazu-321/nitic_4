#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;

struct Node {
    vector<int>    keys;
    vector<Node *> children;
    Node          *parent = nullptr;

    bool is_leaf () const {
        return children.empty ();
    }
};

class TwoThreeTree {
   private:
    Node       *root = nullptr;
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

    static void destroy (Node *node) {
        if (!node) return;
        for (Node *child : node->children) destroy (child);
        delete node;
    }

    static bool partition_count (int total, int parts, int min_part, int max_part, vector<int> &out) {
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

    Node *build_subtree (const vector<int> &arr, int l, int r, int height) {
        int n = r - l;
        if (n == 0) return nullptr;

        Node *node = new Node ();
        if (height == 0) {
            node->keys.insert (node->keys.end (), arr.begin () + l, arr.begin () + r);
            return node;
        }

        int min_child = min_keys_for_height (height - 1);
        int max_child = max_keys_for_height (height - 1);

        struct Option {
            int         key_count = 0;
            vector<int> child_sizes;
            int         score = 0;
        };

        Option best;
        bool   has_best = false;

        auto consider = [&] (int key_count, int child_count) {
            vector<int> child_sizes;
            if (!partition_count (n - key_count, child_count, min_child, max_child, child_sizes)) return;

            int    mn = *min_element (child_sizes.begin (), child_sizes.end ());
            int    mx = *max_element (child_sizes.begin (), child_sizes.end ());
            Option opt{key_count, child_sizes, mx - mn};

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

            Node *left     = build_subtree (arr, l, sep, height - 1);
            Node *right    = build_subtree (arr, sep + 1, r, height - 1);
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

            Node *left     = build_subtree (arr, l, sep1, height - 1);
            Node *middle   = build_subtree (arr, sep1 + 1, sep2, height - 1);
            Node *right    = build_subtree (arr, sep2 + 1, r, height - 1);
            node->children = {left, middle, right};
            left->parent   = node;
            middle->parent = node;
            right->parent  = node;
        }

        return node;
    }

    static bool search_node (Node *node, int key) {
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
        vector<u32string> lines;
        size_t            width    = 0;
        size_t            root_pos = 0;
    };

    static u32string repeat_char (char32_t c, size_t n) {
        return u32string (n, c);
    }

    static string to_utf8 (const u32string &s) {
        wstring_convert<codecvt_utf8<char32_t>, char32_t> conv;
        return conv.to_bytes (s);
    }

    static u32string make_label (const Node *node) {
        string label = "[";
        for (size_t i = 0; i < node->keys.size (); ++i) {
            label += to_string (node->keys[i]);
            if (i + 1 < node->keys.size ()) label += ", ";
        }
        label += "]";
        return u32string (label.begin (), label.end ());
    }

    RenderBox render_box (Node *node) const {
        RenderBox box;
        if (!node) return box;

        u32string label = make_label (node);
        if (node->children.empty ()) {
            box.lines.push_back (label);
            box.width    = label.size ();
            box.root_pos = label.size () / 2;
            return box;
        }

        vector<RenderBox> child_boxes;
        child_boxes.reserve (node->children.size ());
        for (Node *child : node->children) child_boxes.push_back (render_box (child));

        const size_t gap          = child_boxes.size () == 2 ? 1 : 2;
        const size_t label_pad    = 3;
        size_t       label_width  = label.size () + label_pad * 2;
        size_t       label_center = label_width / 2;

        vector<long> rel_lefts (child_boxes.size (), 0);
        if (child_boxes.size () == 1) {
            rel_lefts[0] = 0;
        } else if (child_boxes.size () == 2) {
            rel_lefts[0] = 0;
            rel_lefts[1] = static_cast<long> (child_boxes[0].width) + static_cast<long> (gap);
        } else if (child_boxes.size () == 3) {
            rel_lefts[1] = 0;
            rel_lefts[0] = -static_cast<long> (child_boxes[0].width + gap);
            rel_lefts[2] = static_cast<long> (child_boxes[1].width + gap);
        } else {
            rel_lefts[0] = 0;
            for (size_t i = 1; i < child_boxes.size (); ++i) {
                rel_lefts[i] = rel_lefts[i - 1] + static_cast<long> (child_boxes[i - 1].width) + static_cast<long> (gap);
            }
        }

        vector<long> child_centers (child_boxes.size (), 0);
        for (size_t i = 0; i < child_boxes.size (); ++i) {
            child_centers[i] = rel_lefts[i] + static_cast<long> (child_boxes[i].root_pos);
        }

        long root_center_rel = 0;
        if (child_boxes.size () == 1) {
            root_center_rel = child_centers[0];
        } else if (child_boxes.size () == 2) {
            root_center_rel = (child_centers[0] + child_centers[1]) / 2;
        } else if (child_boxes.size () == 3) {
            root_center_rel = child_centers[1];
        } else {
            root_center_rel = child_centers[child_centers.size () / 2];
        }

        long label_box_left_rel  = root_center_rel - static_cast<long> (label_width / 2);
        long label_box_right_rel = label_box_left_rel + static_cast<long> (label_width - 1);
        long label_x_rel         = label_box_left_rel + static_cast<long> (label_pad);
        long min_left            = min (label_box_left_rel, root_center_rel);
        long max_right           = max (label_box_right_rel + 1, root_center_rel + 1);
        for (size_t i = 0; i < child_boxes.size (); ++i) {
            min_left  = min (min_left, rel_lefts[i]);
            max_right = max (max_right, rel_lefts[i] + static_cast<long> (child_boxes[i].width));
        }

        long   shift   = -min_left;
        size_t width   = static_cast<size_t> (max_right - min_left);
        size_t label_x = static_cast<size_t> (label_x_rel + shift);
        size_t root_x  = static_cast<size_t> (root_center_rel + shift);

        vector<size_t> child_lefts (child_boxes.size (), 0);
        for (size_t i = 0; i < child_boxes.size (); ++i) {
            child_lefts[i] = static_cast<size_t> (rel_lefts[i] + shift);
        }

        size_t left_cap  = child_lefts.front () + child_boxes.front ().root_pos;
        size_t right_cap = child_lefts.back () + child_boxes.back ().root_pos;
        if (left_cap > right_cap) swap (left_cap, right_cap);
        if (label_x <= left_cap) label_x = left_cap + 1;
        size_t label_end = label_x + label.size ();
        if (label_end > right_cap) {
            if (right_cap > label.size ())
                label_x = right_cap - label.size ();
            else
                label_x = left_cap + 1;
            label_end = label_x + label.size ();
        }
        if (label_end > width) width = label_end;
        if (right_cap >= width) width = right_cap + 1;
        box.width    = width;
        box.root_pos = root_x;

        u32string top_row (width, U' ');
        for (size_t x = left_cap + 1; x < right_cap; ++x) top_row[x] = U'━';
        for (size_t i = 0; i < label.size (); ++i) top_row[label_x + i] = label[i];
        top_row[left_cap]  = U'┏';
        top_row[right_cap] = U'┓';
        box.lines.push_back (top_row);

        u32string connect_row (width, U' ');
        if (child_boxes.size () == 3) {
            connect_row[root_x] = U'┃';
        }
        for (size_t i = 0; i < child_boxes.size (); ++i) {
            size_t center = child_lefts[i] + child_boxes[i].root_pos;
            if (center < connect_row.size ()) connect_row[center] = U'┃';
        }
        box.lines.push_back (connect_row);

        size_t child_height = 0;
        for (const RenderBox &child : child_boxes) child_height = max (child_height, child.lines.size ());
        vector<u32string> merged (child_height, u32string (width, U' '));
        for (size_t i = 0; i < child_boxes.size (); ++i) {
            const RenderBox &child = child_boxes[i];
            for (size_t line = 0; line < child.lines.size (); ++line) {
                u32string padded = child.lines[line] + repeat_char (U' ', child.width - child.lines[line].size ());
                merged[line].replace (child_lefts[i], child.width, padded);
            }
        }
        for (const u32string &line : merged) box.lines.push_back (line);

        return box;
    }

    void print_rendered (const RenderBox &box) const {
        for (const u32string &line : box.lines) {
            u32string trimmed = line;
            while (!trimmed.empty () && trimmed.back () == U' ') trimmed.pop_back ();
            cout << to_utf8 (trimmed) << endl;
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

    void print_search_trace (int key) const {
        cout << "Search " << key << " trace" << endl;
        if (!root) {
            cout << "└─[] -> not found" << endl;
            return;
        }
        print_search_trace_node (root, key, 0);
    }

    void print () const {
        if (!root) {
            cout << "[]" << endl;
            return;
        }
        print_rendered (render_box (root));
    }

    void print_search_trace_node (Node *node, int key, int depth) const {
        string indent (static_cast<size_t> (depth) * 2, ' ');
        cout << indent << "└─" << to_utf8 (make_label (node));

        bool found = false;
        for (int k : node->keys) {
            if (key == k) {
                found = true;
                break;
            }
        }
        if (found) {
            cout << " -> found" << endl;
            return;
        }

        if (node->is_leaf ()) {
            cout << " -> not found" << endl;
            return;
        }

        size_t next = 0;
        string move;
        if (node->keys.size () == 1) {
            if (key < node->keys[0]) {
                next = 0;
                move = "left";
            } else {
                next = 1;
                move = "right";
            }
        } else {
            if (key < node->keys[0]) {
                next = 0;
                move = "left";
            } else if (key < node->keys[1]) {
                next = 1;
                move = "middle";
            } else {
                next = 2;
                move = "right";
            }
        }

        cout << " -> go " << move << endl;
        print_search_trace_node (node->children[next], key, depth + 1);
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

    tree.print_search_trace (9);
    cout << "Search 9: " << (tree.search (9) ? "Found" : "Not found") << endl;
    cout << "--------------------------------------------------" << endl;
    tree.print_search_trace (18);
    cout << "Search 18: " << (tree.search (18) ? "Found" : "Not found") << endl;
    cout << "--------------------------------------------------" << endl;

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
