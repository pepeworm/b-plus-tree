#include <bits/stdc++.h>
using namespace std;

enum class NodeType { NODE_ROOT, NODE_INTERNAL, NODE_LEAF };

template <class T> class Node {
private:
	Node<string>* splitLeaf(int rIndex) {
		Node<string>* newSiblingNode = new Node();

		// Set up the new sibling node

		newSiblingNode->type = NodeType::NODE_LEAF;

		// Update leaf nodes linked list

		newSiblingNode->prev = this; // Left pointer of the new node
		newSiblingNode->next = this->next; // Right pointer of the new node
		this->next = newSiblingNode; // Make the original node point to the new node

		if (newSiblingNode->next) { // Check if the original node pointed to another node to its right
			newSiblingNode->next->prev = newSiblingNode; // Make that node point to the new node
		}

		// Transfer keys and vals over to new leaf node

		for (int i = this->size - 1; i >= rIndex; i--) {
			int key = this->keys[i];
			string val = this->vals[i];

			newSiblingNode->set(key, val);
			this->keys.pop_back();
			this->vals.pop_back();
		}

		// Cleanup & set sizes of the current and new nodes

		this->keys.shrink_to_fit();
		this->children.shrink_to_fit();
		this->vals.shrink_to_fit();

		newSiblingNode->size = newSiblingNode->keys.size();
		this->size = this->keys.size();

		return newSiblingNode;
	}

	Node<string>* splitInternal(int rIndex) {
		Node<string>* newSiblingNode = new Node();

		newSiblingNode->type = NodeType::NODE_INTERNAL;
		newSiblingNode->keys.resize(this->size - rIndex - 1);
		newSiblingNode->children.resize(this->size - rIndex);

		// Transfer children over to new internal node

		for (int i = this->size; i > rIndex; i--) {
			newSiblingNode->children[i - rIndex - 1] = this->children[i];
			this->children[i]->parent = newSiblingNode; // Update the parents of child nodes being split off to the new sibling node
			this->children.pop_back();
		}

		// Transfer keys over to the new node

		newSiblingNode->size = newSiblingNode->keys.size();

		for (int i = this->size - 1; i >= this->size - newSiblingNode->size; i--) {
			newSiblingNode->keys[i - (this->size - newSiblingNode->size)] = this->keys[i];
			this->keys.pop_back();
		}

		this->keys.pop_back(); // Since non-leaf nodes don't keep the key being lifted to the parent node
		this->keys.shrink_to_fit();
		this->children.shrink_to_fit();
		this->vals.shrink_to_fit();

		this->size = this->keys.size();

		return newSiblingNode;
	}

public:
	NodeType type;
	Node<string>* parent;
	int size;

	vector<int> keys;

	// For only internal nodes

	vector<Node<string>*> children;

	// For only leaf nodes

	vector<T> vals;
	Node<string>* prev;
	Node<string>* next;

	Node() : parent(nullptr), prev(nullptr), next(nullptr), size(0) {}

	~Node() {
		this->keys.clear();
		this->children.clear();
		this->vals.clear();
		this->prev = nullptr;
		this->next = nullptr;
		this->parent = nullptr;

		return;
	}

	int findKey(int key) {
		int l = 0;
		int r = this->keys.size() - 1;

		while (l <= r) {
			int mid = (l + r) >> 1;

			if (this->keys[mid] == key) {
				return mid;
			} else if (this->keys[mid] < key) {
				l = mid + 1;
			} else {
				r = mid - 1;
			}
		}

		return -1;
	}

	int keyInsertIndex(int key) {
		return upper_bound(this->keys.begin(), this->keys.end(), key) - this->keys.begin();
	}

	int indexOfChild(Node<string>* child) {
		// Find the index in the children vector of a child node

		for (int i = 0; i < this->children.size(); i++) {
			if (this->children[i] == child) {
				return i;
			}
		}

		return -1;
	}

	void removeFromLeaf(int key) {
		int index = this->findKey(key);

		if (index == -1) {
			return;
		}

		this->keys.erase(this->keys.begin() + index);
		this->vals.erase(this->vals.begin() + index);

		this->size--;

		if (this->parent) {
			int cIndex = this->parent->findKey(key); // The index of the current leaf node in the parent node

			if (cIndex == -1) {
				return;
			}

			this->parent->keys[cIndex] = this->keys.front(); // Update the key in the parent node to match the new lowest key in the current leaf node
		}

		return;
	}

	void removeFromInternal(int key) {
		int index = this->findKey(key);

		if (index == -1) {
			return;
		}

		Node<string>* leftMostLeaf = this->children[index];

		while (leftMostLeaf->type != NodeType::NODE_LEAF) {
			leftMostLeaf = leftMostLeaf->children.front();
		}

		this->keys[index] = leftMostLeaf->keys.front();
		this->size--;

		return;
	}

	void borrowFromRightLeaf() {
		Node<string>* next = this->next;
		Node<string>* nextParent = next->parent;

		// Transfer over key

		this->keys.push_back(next->keys.front());
		this->vals.push_back(next->vals.front());
		next->keys.erase(next->keys.begin());
		next->vals.erase(next->vals.begin());

		this->size++;
		next->size--;

		// Update the key in the parent node to the inorder successor (since the inorder predecessor's smallest key doesn't change)

		nextParent->keys[nextParent->indexOfChild(next) - 1] = next->keys.front();

		return;
	}

	void borrowFromLeftLeaf() {
		Node<string>* prev = this->prev;
		Node<string>* parent = this->parent;

		// Transfer over key

		this->keys.insert(this->keys.begin(), prev->keys.back());
		this->vals.insert(this->vals.begin(), prev->vals.back());
		prev->keys.pop_back();
		prev->vals.pop_back();

		this->size++;
		prev->size--;

		// Update the key in the parent node to the new inorder successor

		parent->keys[parent->indexOfChild(this) - 1] = this->keys.front();

		return;
	}

	void mergeWithRightLeaf() {
		Node<string>* next = this->next;
		Node<string>* nextParent = next->parent;

		// Merge the current node with the next node (transfer keys and values over into the current node)

		for (int i = 0; i < next->size; i++) {
			int key = next->keys[i];
			string val = next->vals[i];

			this->set(key, val);
		}

		// Pointer management

		this->next = next->next;

		if (this->next) {
			this->next->prev = this;
		}

		// Update keys of the next node's parent node

		int childIndex = nextParent->indexOfChild(next);

		nextParent->keys.erase(nextParent->keys.begin() + childIndex - 1);
		nextParent->children.erase(nextParent->children.begin() + childIndex);

		nextParent->size--;

		// Delete the now-unused next node

		delete next;

		return;
	}

	void mergeWithLeftLeaf() {
		Node<string>* prev = this->prev;
		Node<string>* parent = this->parent;

		// Merge the current node with the prev node (transfer keys and values over into the previous node)

		for (int i = 0; i < this->size; i++) {
			int key = this->keys[i];
			string val = this->vals[i];

			prev->set(key, val);
		}

		// Pointer management

		prev->next = this->next;

		if (prev->next) {
			prev->next->prev = prev;
		}

		// Update keys of the current node's parent node

		int childIndex = parent->indexOfChild(this);

		parent->keys.erase(parent->keys.begin() + childIndex - 1);
		parent->children.erase(parent->children.begin() + childIndex);

		parent->size--;

		// Delete the now-unused current node

		delete this;

		return;
	}

	void borrowFromRightInternal(Node<string>* next) {
		Node<string>* parent = this->parent;
		int childIndex = parent->indexOfChild(this);

		// Transfer the key over to the current node

		this->keys.push_back(parent->keys[childIndex]);
		parent->keys[childIndex] = next->keys.front();
		next->keys.erase(this->keys.begin());

		this->size++;
		next->size--;

		// Update pointers

		this->children.push_back(next->children.front());
		next->children.erase(next->children.begin());
		this->children.back()->parent = this;

		return;
	}

	void borrowFromLeftInternal(Node<string>* prev) {
		Node<string>* parent = this->parent;
		int childIndex = parent->indexOfChild(this);

		// Transfer the key over to the current node

		this->keys.insert(this->keys.begin(), parent->keys[childIndex - 1]);
		parent->keys[childIndex - 1] = prev->keys.back();
		prev->keys.pop_back();

		this->size++;
		prev->size--;

		// Update pointers

		this->children.insert(this->children.begin(), prev->children.back());
		prev->children.pop_back();
		this->children.front()->parent = this;

		return;
	}

	void mergeWithRightInternal(Node<string>* next) {
		Node<string>* parent = this->parent;
		int childIndex = parent->indexOfChild(this);

		// Transfer key from the parent node to the current node

		this->keys.push_back(parent->keys[childIndex]);
		parent->keys.erase(parent->keys.begin() + childIndex);
		parent->children.erase(parent->children.begin() + childIndex + 1); // Delete the next node from the parent

		this->size += next->size + 1; // All keys from the next node + the key from the parent
		parent->size--;

		// Merge the current node with the next node (transfer keys and children over into the current node)

		for (int key : next->keys) {
			this->keys.push_back(key);
		}

		for (Node<string>* child : next->children) {
			this->children.push_back(child);
			child->parent = this;
		}

		// Delete the now-unused next node

		delete next;

		return;
	}

	void mergeWithLeftInternal(Node<string>* prev) {
		Node<string>* parent = this->parent;
		int childIndex = parent->indexOfChild(this);

		// Transfer key from the parent node to the prev node

		prev->keys.push_back(parent->keys[childIndex - 1]);
		parent->keys.erase(parent->keys.begin() + childIndex - 1);
		parent->children.erase(parent->children.begin() + childIndex);

		prev->size += this->size + 1;
		parent->size--;

		// Merge the current node with the prev node (transfer keys and children over into the prev node)

		for (int key : this->keys) {
			prev->keys.push_back(key);
		}

		for (Node<string>* child : this->children) {
			prev->children.push_back(child);
			child->parent = prev;
		}

		// Delete the now-unused curr node

		delete this;

		return;
	}

	void set(int key, string val) {
		int keyIndex = this->findKey(key);

		if (keyIndex != -1) {
			this->vals[keyIndex] = val;

			return;
		}

		Node<string>* curr = this;
		int insIndex = curr->keyInsertIndex(key);

		curr->keys.insert(curr->keys.begin() + insIndex, key);
		curr->vals.insert(curr->vals.begin() + insIndex, val);
		curr->size++;

		return;
	}

	Node<string>* splitNode() {
		int rIndex = this->size >> 1; // Right split start index: [rIndex, Node->size]; Left split end index: [0, rIndex)
		int newParentKey = this->keys[rIndex];

		Node<string>* siblingNode;

		if (this->type == NodeType::NODE_LEAF) {
			siblingNode = this->splitLeaf(rIndex);
		} else {
			siblingNode = this->splitInternal(rIndex);
		}

		// Make changes to the parent node

		if (this->parent) {
			Node<string>* parentNode = this->parent;

			int index = parentNode->keyInsertIndex(newParentKey);
			parentNode->keys.insert(parentNode->keys.begin() + index, newParentKey);
			parentNode->size++;

			if (index > 0) {
				parentNode->children.insert(parentNode->children.begin() + index + 1, siblingNode);
			} else {
				parentNode->children.insert(parentNode->children.begin(), this);
				parentNode->children.insert(parentNode->children.begin() + 1, siblingNode);
			}

			siblingNode->parent = parentNode;
		} else {
			Node<string>* newRootNode = new Node();

			newRootNode->type = NodeType::NODE_ROOT;
			newRootNode->keys.push_back(newParentKey);
			newRootNode->size = 1;
			newRootNode->children.push_back(this);
			newRootNode->children.push_back(siblingNode);

			// Update the nodes at the original level

			if (this->type == NodeType::NODE_ROOT) {
				this->type = NodeType::NODE_INTERNAL;
			}

			this->parent = newRootNode;
			siblingNode->parent = newRootNode;

			return newRootNode;
		}

		return nullptr;
	}
};

class BTree {
private:
	Node<string>* findLeaf(int key) {
		Node<string>* curr = this->root;

		// Descend to correct leaf node

		while (curr->type != NodeType::NODE_LEAF) {
			int index = curr->keyInsertIndex(key);
			curr = curr->children[index];
		}

		return curr;
	}

public:
	int deg;
	int depth = 1;
	Node<string>* root;

	BTree(int deg) : deg(deg) {
		this->root = new Node<string>();
		this->root->type = NodeType::NODE_LEAF;

		return;
	};

	void printTree() {
		this->printTree(this->root, "", true);

		return;
	}

	void printTree(Node<string>* node, string prefix, bool last) {
		cout << prefix << "├ [";

		for (int i = 0; i < node->keys.size(); i++) {
			cout << node->keys[i];
			if (i != node->keys.size() - 1) {
				cout << ", ";
			}
		}

		cout << "]" << endl;

		prefix += last ? "   " : "╎  ";

		if (!(node->type == NodeType::NODE_LEAF)) {
			for (int i = 0; i < node->children.size(); i++) {
				bool last = (i == node->children.size() - 1);
				printTree(node->children[i], prefix, last);
			}
		}
	}

	void set(int key, string val) {
		Node<string>* curr = this->findLeaf(key);
		int index = curr->findKey(key);

		curr->set(key, val);

		if (index != -1) {
			return;
		}

		while (curr && curr->size >= deg) {
			Node<string>* newRootNode = curr->splitNode();

			if (newRootNode) {
				this->depth++;
				this->root = newRootNode;
			}

			curr = curr->parent;
		}

		return;
	}

	void remove(int key) {
		Node<string>* curr = this->findLeaf(key);
		int index = curr->findKey(key);

		if (index != -1) {
			this->remove(key, curr);
		}

		return;
	}

	void remove(int key, Node<string>* curr) {
		int minCapacity = this->deg >> 1;

		if (curr->type == NodeType::NODE_LEAF) {
			curr->removeFromLeaf(key);
		} else {
			curr->removeFromInternal(key);
		}

		// Check if curr node is underfull as a result

		if (curr->size < minCapacity) {
			if (curr->type == NodeType::NODE_ROOT) { // If the underfull node is the root node
				if (!curr->size && !curr->children.empty()) {
					curr = curr->children[0];
					delete curr->parent;

					curr->parent = nullptr;
					this->root = curr;
					this->depth--;

					if (this->depth > 1) {
						curr->type = NodeType::NODE_ROOT;
					} else {
						curr->type = NodeType::NODE_LEAF;
					}
				}
			} else if (curr->type == NodeType::NODE_INTERNAL) { // If the underfull node is the internal node
				Node<string>* parent = curr->parent;
				int currChildIndex = parent->indexOfChild(curr);

				Node<string>* next = nullptr;
				Node<string>* prev = nullptr;

				if (parent->children.size() > currChildIndex + 1) {
					next = parent->children[currChildIndex + 1];
				}

				if (currChildIndex) {
					prev = parent->children[currChildIndex - 1];
				}

				if (next && next->parent == parent && next->size > minCapacity) {
					curr->borrowFromRightInternal(next);
				} else if (prev && prev->parent == parent && prev->size > minCapacity) {
					curr->borrowFromLeftInternal(prev);
				} else if (next && next->parent == parent && next->size <= minCapacity) {
					curr->mergeWithRightInternal(next);
				} else if (prev && prev->parent == parent && prev->size <= minCapacity) {
					curr->mergeWithLeftInternal(prev);
					curr = prev;
				}
			} else { // If the underfull node is the leaf node
				Node<string>* parent = curr->parent;
				Node<string>* next = curr->next;
				Node<string>* prev = curr->prev;

				if (next && next->parent == parent && next->size > minCapacity) {
					curr->borrowFromRightLeaf();
				} else if (prev && prev->parent == parent && prev->size > minCapacity) {
					curr->borrowFromLeftLeaf();
				} else if (next && next->parent == parent && next->size <= minCapacity) {
					curr->mergeWithRightLeaf();
				} else if (prev && prev->parent == parent && prev->size <= minCapacity) {
					curr->mergeWithLeftLeaf();
					curr = prev;
				}
			}
		}

		if (curr->parent) {
			this->remove(key, curr->parent);
		}

		return;
	}
};

int main() {
	BTree* btree = new BTree(6);
	btree->set(1, "one");
	btree->set(2, "two");
	btree->set(3, "three");
	btree->set(4, "four");
	btree->set(5, "five");
	btree->set(6, "six");
	btree->set(7, "seven");
	btree->set(9, "nine");
	btree->set(11, "eleven");
	btree->set(8, "eight");
	btree->set(10, "ten");

	btree->printTree();

	cout << '\n';

	btree->remove(1);
	btree->remove(5);
	btree->remove(3);
	btree->remove(8);

	btree->printTree();

	return 0;
}