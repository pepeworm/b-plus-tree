#include <bits/stdc++.h>
using namespace std;

enum class NodeType { NODE_ROOT, NODE_INTERNAL, NODE_LEAF };

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
		this->ptr.resize(2, nullptr);
		this->size = 0;

		return;
	}
};

class BTree {
private:
	int deg;
	int depth = 0;
	Node* root;

	
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
			curr = this->search(key);
		} else {
			curr = this->root;
		}

		// TODO: move all this stuff into an insert function inside Node

		int idx = upper_bound(curr->keys.begin(), curr->keys.end(), key) - curr->keys.begin();
		curr->keys.insert(curr->keys.begin() + idx, key);
		curr->vals.insert(curr->vals.begin() + idx, val);
		curr->size++;

		// Split node

		if (curr->size >= this->deg) {
			int rIdx = this->deg >> 1; // Right split start index: [rIdx, Node->size]; Left split end index: [0, rIdx)
			int rIdxKey = curr->keys[rIdx]; // Key at position rIdx
			
			Node* newLeaf = new Node();
			newLeaf->type = NodeType::NODE_LEAF;

			// Pointer management

			newLeaf->ptr[0] = curr;
			newLeaf->ptr[1] = curr->ptr[1];
			
			if (curr->ptr[1]) {
				curr->ptr[1]->ptr[0] = newLeaf;
			}
			
			curr->ptr[1] = newLeaf;

			// Transfer keys & vals over to new leaf node

			// TODO: move keys[] and vals[] into a new KV class, possibly

			newLeaf->keys.resize(this->deg - rIdx);
			newLeaf->vals.resize(this->deg - rIdx);

			for (int i = this->deg - rIdx; i >= rIdx; i--) {
				newLeaf->keys[i - 1] = curr->keys[i];
				newLeaf->vals[i - 1] = curr->vals[i];

				curr->keys.pop_back();
				curr->vals.pop_back();
			}

			if (this->depth > 1) {
				// Shift level

				
			} else {
				Node* newRoot = new Node();
				newRoot->type = NodeType::NODE_ROOT;
				newRoot->keys.push_back(rIdxKey);
				
				// Pointer management

				newRoot->ptr[0] = curr;
				newRoot->ptr[1] = newLeaf;
				curr->parent = newRoot;
				newLeaf->parent = newRoot;

				this->root = newRoot;
				this->depth++;
			}
		}

		return;
	}

	Node* search(int key) {
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
				return curr;
			} else if (curr->keys[mid] < key) {
				l = mid + 1;
			} else {
				r = mid - 1;
			}
		}

		return nullptr;
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
				for (int i = 0; i < curr->size; i++) {
					cout << curr->keys[i] << " (" << curr->vals[i] << ") ";
				}

				cout << '\n';
			} else {
				for (Node* p : curr->ptr) {
					q.push(p);
				}
				
				for (int k : curr->keys) {
					cout << k << ' ';
				}

				cout << '\n';
			}

		}

		return;
	}
};

int main() {
	BTree* btree = new BTree(3);
	btree->ins(2, "two");
	btree->ins(4, "four");
	btree->ins(3, "three");
	btree->printTree();

	return 0;
}