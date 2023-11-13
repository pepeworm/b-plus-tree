#include <bits/stdc++.h>
using namespace std;

enum class NodeType { NODE_ROOT, NODE_INTERNAL, NODE_LEAF };

// TODO change a node into a binary search tree or deque?

class Node {
public:
	NodeType type;
	Node* parent;
	int size; // Number of elements inside the node

	vector<int> keys;
	vector<Node*> ptr;

	vector<string> vals; // For leaf nodes

	Node() {
		this->type = NodeType::NODE_INTERNAL;
		this->parent = nullptr;
		this->ptr.resize(2, nullptr); // Leaf nodes only contain 2 pointers
		this->size = 0;

		return;
	}

	~Node() {
		this->keys.clear();
		this->ptr.clear();
		this->vals.clear();

		return;
	}
};

class BTree {
private:
	int deg;
	int depth = 0;
	Node* root;

	void nodeSplit(Node* curr) {
		if (curr->size < this->deg) {
			return;
		}

		int rIdx = curr->size >> 1; // Right split start index: [rIdx, Node->size]; Left split end index: [0, rIdx)
		int rIdxKey = curr->keys[rIdx];

		Node* newSiblingNode = new Node();

		// Set up the new sibling node

		if (curr->type == NodeType::NODE_LEAF) {
			newSiblingNode->type = NodeType::NODE_LEAF;
			newSiblingNode->keys.resize(curr->size - rIdx);
			newSiblingNode->vals.resize(curr->size - rIdx);

			// Update leaf nodes linked list

			newSiblingNode->ptr[0] = curr; // Left pointer of the new node
			newSiblingNode->ptr[1] = curr->ptr[1]; // Right pointer of the new node
			curr->ptr[1] = newSiblingNode; // Make the original node point to the new node

			if (newSiblingNode->ptr[1]) { // Check if the original node pointed to another node to its right
				newSiblingNode->ptr[1]->ptr[0] = newSiblingNode; // Make that node point to the new node
			}

			// Transfer vals over to new leaf node

			// TODO: move keys[] and vals[] into a new KV class, possibly

			for (int i = curr->size - 1; i >= rIdx; i--) {
				newSiblingNode->vals[i - rIdx] = curr->vals[i];
				curr->vals.pop_back();
			}
		} else {
			newSiblingNode->type = NodeType::NODE_INTERNAL;
			newSiblingNode->keys.resize(curr->size - rIdx - 1);
			newSiblingNode->ptr.resize(curr->size - rIdx);

			// Transfer ptr over to new internal node

			for (int i = curr->size; i > rIdx; i--) {
				newSiblingNode->ptr[i - rIdx - 1] = curr->ptr[i];
				curr->ptr[i]->parent = newSiblingNode; // Update the parents of child nodes 
				
				curr->ptr.pop_back();
			}
		}

		// Transfer keys over to the new node

		for (int i = curr->size - 1; i >= curr->size - newSiblingNode->keys.size(); i--) {
			newSiblingNode->keys[i - (curr->size - newSiblingNode->keys.size())] = curr->keys[i];
			curr->keys.pop_back();
		}

		if (curr->type != NodeType::NODE_LEAF) {
			curr->keys.pop_back();
		}

		curr->keys.shrink_to_fit();
		curr->ptr.shrink_to_fit();
		curr->vals.shrink_to_fit();

		curr->size = curr->keys.size();
		newSiblingNode->size = newSiblingNode->keys.size();

		// Make changes to the parent node

		if (curr->parent) {
			Node* parentNode = curr->parent;

			int idx = upper_bound(parentNode->keys.begin(), parentNode->keys.end(), rIdxKey) - parentNode->keys.begin();
			parentNode->keys.insert(parentNode->keys.begin() + idx, rIdxKey);
			parentNode->size++;

			if (idx > 0) {
				parentNode->ptr.insert(parentNode->ptr.begin() + idx + 1, newSiblingNode);
			} else {
				parentNode->ptr.insert(parentNode->ptr.begin(), curr);
				parentNode->ptr.insert(parentNode->ptr.begin() + 1, newSiblingNode);
			}

			newSiblingNode->parent = parentNode;
		} else {
			Node* newRootNode = new Node();

			newRootNode->type = NodeType::NODE_ROOT;
			newRootNode->keys.push_back(rIdxKey);
			newRootNode->size = 1;
			newRootNode->ptr[0] = curr;
			newRootNode->ptr[1] = newSiblingNode;

			this->root = newRootNode; // Set new B-Tree root
			this->depth++;

			// Update the nodes at the original level

			if (curr->type == NodeType::NODE_ROOT) {
				curr->type = NodeType::NODE_INTERNAL;
			}

			curr->parent = newRootNode;
			newSiblingNode->parent = newRootNode;
		}

		return nodeSplit(curr->parent);
	}

public:
	BTree(int deg) : deg(deg){};

	void del() {}

	void ins(int key, string val) {
		if (!this->root) {
			this->root = new Node();
			this->root->type = NodeType::NODE_LEAF;

			// TODO: change this

			this->depth++;
		}

		Node* curr;

		if (this->depth > 1) {
			curr = this->search(key).first;
		} else {
			curr = this->root;
		}

		if (!curr) {
			cout << "Search function failed" << '\n';
		}

		// TODO: move all this stuff into an insert function inside Node

		int idx = upper_bound(curr->keys.begin(), curr->keys.end(), key) - curr->keys.begin();
		curr->keys.insert(curr->keys.begin() + idx, key);
		curr->vals.insert(curr->vals.begin() + idx, val);
		curr->size++;

		// Split node

		this->nodeSplit(curr);

		return;
	}

	pair<Node*, int> search(int key) { // <Node, keyIdx (-1 is not found)>
		// TODO: currently, this just returns the leaf node; make it such that it returns the leaf node / the actual key + data depending on what is required

		Node* curr = this->root;

		// Descend to correct leaf node

		while (curr->type != NodeType::NODE_LEAF) {
			int idx = upper_bound(curr->keys.begin(), curr->keys.end(), key) - curr->keys.begin();
			curr = curr->ptr[idx];
		}

		// Binary search in the leaf node

		int l = 0;
		int r = curr->size - 1;

		while (l <= r) {
			int mid = (l + r) >> 1;

			if (curr->keys[mid] == key) {
				return {curr, mid};
			} else if (curr->keys[mid] < key) {
				l = mid + 1;
			} else {
				r = mid - 1;
			}
		}

		return {curr, -1};
	}

	void printTree() {
		if (!this->root) {
			cout << "No nodes" << '\n';

			return;
		}

		queue<Node*> q;
		q.push(this->root);

		while (!q.empty()) {
			Node* curr = q.front();
			q.pop();

			if (curr->type == NodeType::NODE_LEAF) {
				cout << "(leaf) |";

				for (int i = 0; i < curr->size; i++) {
					cout << curr->keys[i] << " (" << curr->vals[i] << ") ";
				}

				cout << "|\n";
			} else {
				for (Node* p : curr->ptr) {
					q.push(p);
				}

				if (curr->type == NodeType::NODE_ROOT) {
					cout << "(root) ";
				} else {
					cout << "(int ) ";
				}

				cout << '|';

				for (int k : curr->keys) {
					cout << k << ' ';
				}

				cout << "|\n";
			}
		}

		return;
	}
};

int main() {
	BTree* btree = new BTree(3);
	btree->ins(1, "one");
	btree->ins(2, "two");
	btree->ins(3, "three");
	btree->ins(4, "four");
	btree->ins(5, "five");
	btree->ins(2, "dup");

	btree->printTree();
	
	pair<Node*, int> pos = btree->search(7);

	for (int i = 0; i < pos.first->size; i++) {
		cout << pos.first->keys[i] << " (" << pos.first->vals[i] << ") ";
	}

	cout << pos.second << '\n';
	
	return 0;
}