#include <iostream>
#include <algorithm>
#include <map>
#include <string>
#include <vector>
using namespace std;

class Node {
public:
    string label;
    bool is_end;
    map<char, Node*> children;

    Node(const string& l = "", bool end = false) : label(l), is_end(end) {}
};

class PatriciaTree {
private:
    Node* root;

    struct RenderedTree {
        vector<string> lines;
        int width;
        int root_x;
    };

    static void trimRight(string& s) {
        while (!s.empty() && s.back() == ' ') s.pop_back();
    }

    RenderedTree renderTree(Node* node, bool is_root) const {
        string label = is_root ? "ROOT" : node->label;
        string marker = node->is_end ? "[end]" : "|";

        if (node->children.empty()) {
            int width = max(static_cast<int>(label.size()), static_cast<int>(marker.size()));
            vector<string> lines(2, string(width, ' '));
            int label_start = (width - static_cast<int>(label.size())) / 2;
            int marker_start = (width - static_cast<int>(marker.size())) / 2;
            for (int i = 0; i < static_cast<int>(label.size()); ++i) lines[0][label_start + i] = label[i];
            for (int i = 0; i < static_cast<int>(marker.size()); ++i) lines[1][marker_start + i] = marker[i];
            return {lines, width, label_start + static_cast<int>(label.size() / 2)};
        }

        vector<RenderedTree> children;
        children.reserve(node->children.size());
        for (auto& [_, child] : node->children) {
            children.push_back(renderTree(child, false));
        }

        const int gap = 4;
        int children_width = 0;
        int max_child_height = 0;
        for (size_t i = 0; i < children.size(); ++i) {
            children_width += children[i].width;
            if (i + 1 < children.size()) children_width += gap;
            max_child_height = max(max_child_height, static_cast<int>(children[i].lines.size()));
        }

        int width = max({static_cast<int>(label.size()), static_cast<int>(marker.size()), children_width});
        int label_start = (width - static_cast<int>(label.size())) / 2;
        int marker_start = (width - static_cast<int>(marker.size())) / 2;
        int children_start = (width - children_width) / 2;
        vector<string> canvas(3 + max_child_height, string(width, ' '));

        auto put = [&](int row, int col, char ch) {
            if (row >= 0 && row < static_cast<int>(canvas.size()) && col >= 0 && col < width) {
                canvas[row][col] = ch;
            }
        };

        for (int i = 0; i < static_cast<int>(label.size()); ++i) put(0, label_start + i, label[i]);
        for (int i = 0; i < static_cast<int>(marker.size()); ++i) put(1, marker_start + i, marker[i]);

        int offset = children_start;
        vector<int> child_centers;
        child_centers.reserve(children.size());
        for (const auto& child : children) {
            int center = offset + child.root_x;
            child_centers.push_back(center);
            for (int row = 0; row < static_cast<int>(child.lines.size()); ++row) {
                const string& line = child.lines[row];
                for (int col = 0; col < static_cast<int>(line.size()); ++col) {
                    if (line[col] != ' ') put(row + 3, offset + col, line[col]);
                }
            }
            offset += child.width + gap;
        }

        if (child_centers.size() == 1) {
            put(2, child_centers[0], '|');
        } else if (child_centers.size() == 2) {
            put(2, child_centers[0], '/');
            put(2, child_centers[1], static_cast<char>(92));
        } else {
            for (size_t i = 0; i < child_centers.size(); ++i) {
                put(2, child_centers[i], i == 0 ? '/' : (i + 1 == child_centers.size() ? static_cast<char>(92) : '|'));
            }
        }

        for (auto& line : canvas) trimRight(line);
        while (!canvas.empty() && canvas.back().empty()) canvas.pop_back();
        return {canvas, width, label_start + static_cast<int>(label.size() / 2)};
    }

    bool erase(Node* node, const string& word, int depth) {
        for (auto it = node->children.begin(); it != node->children.end(); ++it) {
            char key = it->first;
            Node* child = it->second;
            const string& label = child->label;
            int len = 0;
            while (depth + len < word.size() && len < label.size() &&
                   word[depth + len] == label[len]) {
                ++len;
            }

            if (len == label.size()) {
                if (depth + len == word.size()) {
                    if (!child->is_end) return false;
                    child->is_end = false;
                    
                    // If this node has no children and is not an end node, delete it
                    if (child->children.empty()) {
                        delete child;
                        node->children.erase(key);
                        return true;
                    }
                    
                    // If this node has only one child and is not an end node, merge with child
                    if (child->children.size() == 1 && !child->is_end) {
                        auto child_it = child->children.begin();
                        Node* grandchild = child_it->second;
                        grandchild->label = child->label + grandchild->label;
                        node->children[key] = grandchild;
                        delete child;
                    }
                    
                    return true;
                }
                bool result = erase(child, word, depth + len);
                
                // After recursive deletion, check if child needs cleanup
                if (result && !child->is_end && child->children.empty()) {
                    delete child;
                    node->children.erase(key);
                } else if (result && !child->is_end && child->children.size() == 1) {
                    // Merge with single child
                    auto child_it = child->children.begin();
                    Node* grandchild = child_it->second;
                    grandchild->label = child->label + grandchild->label;
                    node->children[key] = grandchild;
                    delete child;
                }
                
                return result;
            }
        }
        return false;
    }

public:
    PatriciaTree() {
        root = new Node();
    }
    
    ~PatriciaTree() {
        deleteTree(root);
    }

private:
    void deleteTree(Node* node) {
        if (node) {
            for (auto& [_, child] : node->children) {
                deleteTree(child);
            }
            delete node;
        }
    }

public:
    void insert(const string& word) {
        Node* node = root;
        int i = 0;
        while (i < word.size()) {
            char ch = word[i];
            if (!node->children.count(ch)) {
                node->children[ch] = new Node(word.substr(i), true);
                return;
            }

            Node* child = node->children[ch];
            const string& label = child->label;
            int j = 0;
            while (i + j < word.size() && j < label.size() && word[i + j] == label[j]) {
                ++j;
            }

            if (j == label.size()) {
                node = child;
                i += j;
                continue;
            }

            Node* split = new Node(label.substr(0, j), false);
            child->label = label.substr(j);
            split->children[child->label[0]] = child;
            node->children[ch] = split;

            if (i + j < word.size()) {
                split->children[word[i + j]] = new Node(word.substr(i + j), true);
            } else {
                split->is_end = true;
            }
            return;
        }
        node->is_end = true;
    }

    bool search(const string& word) const {
        Node* node = root;
        int i = 0;
        while (i < word.size()) {
            char ch = word[i];
            if (!node->children.count(ch)) return false;

            Node* child = node->children.at(ch);
            const string& label = child->label;
            int j = 0;
            while (i + j < word.size() && j < label.size() && word[i + j] == label[j]) {
                ++j;
            }
            if (j < label.size()) return false;
            node = child;
            i += j;
        }
        return node->is_end;
    }

    bool startsWith(const string& prefix) const {
        Node* node = root;
        int i = 0;
        while (i < prefix.size()) {
            char ch = prefix[i];
            if (!node->children.count(ch)) return false;

            Node* child = node->children.at(ch);
            const string& label = child->label;
            int j = 0;
            while (i + j < prefix.size() && j < label.size() && prefix[i + j] == label[j]) {
                ++j;
            }
            if (j < label.size() && i + j < prefix.size()) return false;
            node = child;
            i += j;
        }
        return true;
    }

    bool erase(const string& word) {
        return erase(root, word, 0);
    }

    void print() const {
        cout << "Patricia Tree:" << endl;
        auto rendered = renderTree(root, true);
        for (const auto& line : rendered.lines) {
            cout << line << endl;
        }
    }
};

int main() {
    PatriciaTree tree;
    vector<string> insert_words = {"man", "mane", "dry", "dried", "mat"};
    vector<string> search_words = {"mane", "many", "drie"};
    vector<string> delete_words = {"dried", "mat", "dry", "man", "mane"};

    for (const auto& w : insert_words) {
        cout << "Inserting: " << w << endl;
        tree.insert(w);

        cout << endl;
        tree.print();
        cout << "--------------------------------------------------" << endl;
    }

    cout << "\nSearch Results:" << endl;
    for (const auto& w : search_words) {
        cout << w << ": " << tree.search(w) << " (startsWith: " << tree.startsWith(w) << ")\n";
    }

    for (const auto& w : delete_words) {
        cout << "Deleting: " << w << endl;
        tree.erase(w);

        cout << endl;
        tree.print();
        cout << "--------------------------------------------------" << endl;
    }
    return 0;
}