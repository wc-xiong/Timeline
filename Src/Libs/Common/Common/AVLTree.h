#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>

// AVL树节点模板类
template <typename T>
struct AVLNode {
	T data;                // 节点数据，支持任意可比较的数据类型
	int height;            // 节点高度，用于平衡判断
	std::shared_ptr<AVLNode<T>> left;  // 左孩子（智能指针自动管理内存）
	std::shared_ptr<AVLNode<T>> right; // 右孩子

	// 构造函数
	explicit AVLNode(const T& val) : data(val), height(1), left(nullptr), right(nullptr) {}
	explicit AVLNode(T&& val) : data(std::move(val)), height(1), left(nullptr), right(nullptr) {}
};

// AVL树模板类
template <typename T, typename Comparator = std::less<T>>
class AVLTree {
private:
	std::shared_ptr<AVLNode<T>> root_;  // 根节点
	Comparator comp_;                   // 比较器，支持自定义比较规则

	// 获取节点高度（空节点高度为0）
	int getHeight(const std::shared_ptr<AVLNode<T>>& node) const {
		return node ? node->height : 0;
	}

	// 更新节点高度（当前节点高度 = 左右孩子高度最大值 + 1）
	void updateHeight(std::shared_ptr<AVLNode<T>>& node) {
		if (node) {
			node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
		}
	}

	// 获取节点平衡因子（左子树高度 - 右子树高度）
	int getBalanceFactor(const std::shared_ptr<AVLNode<T>>& node) const {
		return node ? (getHeight(node->left) - getHeight(node->right)) : 0;
	}

	// 右旋转（处理左子树过高的情况，LL型）
	std::shared_ptr<AVLNode<T>> rightRotate(std::shared_ptr<AVLNode<T>> y) {
		std::shared_ptr<AVLNode<T>> x = std::move(y->left);
		std::shared_ptr<AVLNode<T>> T2 = std::move(x->right);

		// 执行旋转
		x->right = std::move(y);
		x->right->left = std::move(T2);

		// 更新节点高度
		updateHeight(x->right);
		updateHeight(x);

		return x; // 返回新的根节点
	}

	// 左旋转（处理右子树过高的情况，RR型）
	std::shared_ptr<AVLNode<T>> leftRotate(std::shared_ptr<AVLNode<T>> x) {
		std::shared_ptr<AVLNode<T>> y = std::move(x->right);
		std::shared_ptr<AVLNode<T>> T2 = std::move(y->left);

		// 执行旋转
		y->left = std::move(x);
		y->left->right = std::move(T2);

		// 更新节点高度
		updateHeight(y->left);
		updateHeight(y);

		return y; // 返回新的根节点
	}

	// 递归插入节点（内部实现）
	std::shared_ptr<AVLNode<T>> insertHelper(std::shared_ptr<AVLNode<T>> node, const T& val) {
		// 1. 执行普通二叉搜索树的插入
		if (!node) {
			return std::make_unique<AVLNode<T>>(val);
		}

		if (comp_(val, node->data)) {
			node->left = insertHelper(std::move(node->left), val);
		}
		else if (comp_(node->data, val)) {
			node->right = insertHelper(std::move(node->right), val);
		}
		else {
			// 不允许插入重复元素（可根据需求修改）
			return node;
		}

		// 2. 更新当前节点高度
		updateHeight(node);

		// 3. 获取平衡因子，判断是否需要平衡调整
		int balance = getBalanceFactor(node);

		// 四种不平衡情况处理
		// 情况1：LL型（左左），右旋转
		if (balance > 1 && comp_(val, node->left->data)) {
			return rightRotate(std::move(node));
		}

		// 情况2：RR型（右右），左旋转
		if (balance < -1 && comp_(node->right->data, val)) {
			return leftRotate(std::move(node));
		}

		// 情况3：LR型（左右），先左旋转再右旋转
		if (balance > 1 && comp_(node->left->data, val)) {
			node->left = leftRotate(std::move(node->left));
			return rightRotate(std::move(node));
		}

		// 情况4：RL型（右左），先右旋转再左旋转
		if (balance < -1 && comp_(val, node->right->data)) {
			node->right = rightRotate(std::move(node->right));
			return leftRotate(std::move(node));
		}

		// 平衡状态，返回当前节点
		return node;
	}

	// 递归查找节点（内部实现）
	bool findHelper(const std::shared_ptr<AVLNode<T>>& node, const T& val) const {
		if (!node) {
			return false; // 未找到
		}

		if (comp_(val, node->data)) {
			return findHelper(node->left, val);
		}
		else if (comp_(node->data, val)) {
			return findHelper(node->right, val);
		}
		else {
			return true; // 找到
		}
	}
	
	// 递归按值查找对象（内部实现）
	const std::shared_ptr<AVLNode<T>> getNodeHelper(const std::shared_ptr<AVLNode<T>>& node, const T& val) const {
		if (!node) {
			return nullptr; // 未找到
		}

		if (comp_(val, node->data)) {
			return getNodeHelper(node->left, val);
		}
		else if (comp_(node->data, val)) {
			return getNodeHelper(node->right, val);
		}
		else {
			return node; // 找到
		}
	}

	// 找到以node为根的最小节点（用于删除操作）
	AVLNode<T>* findMinNode(std::shared_ptr<AVLNode<T>>& node) {
		AVLNode<T>* current = node.get();
		while (current && current->left) {
			current = current->left.get();
		}
		return current;
	}

	// 递归删除节点（内部实现）
	std::shared_ptr<AVLNode<T>> deleteHelper(std::shared_ptr<AVLNode<T>> node, const T& val) {
		// 1. 执行普通二叉搜索树的删除
		if (!node) {
			return node; // 未找到要删除的节点
		}

		if (comp_(val, node->data)) {
			node->left = deleteHelper(std::move(node->left), val);
		}
		else if (comp_(node->data, val)) {
			node->right = deleteHelper(std::move(node->right), val);
		}
		else {
			// 找到要删除的节点，处理三种情况
			// 情况1：叶子节点或只有右孩子
			if (!node->left) {
				return std::move(node->right);
			}
			// 情况2：只有左孩子
			else if (!node->right) {
				return std::move(node->left);
			}
			// 情况3：有两个孩子，找到右子树最小节点替代当前节点
			else {
				AVLNode<T>* minNode = findMinNode(node->right);
				node->data = minNode->data; // 复制最小节点数据
				// 删除右子树中的最小节点
				node->right = deleteHelper(std::move(node->right), minNode->data);
			}
		}

		// 2. 更新当前节点高度
		updateHeight(node);

		// 3. 获取平衡因子，判断是否需要平衡调整
		int balance = getBalanceFactor(node);

		// 四种不平衡情况处理（与插入逻辑一致）
		// 情况1：LL型
		if (balance > 1 && getBalanceFactor(node->left) >= 0) {
			return rightRotate(std::move(node));
		}

		// 情况2：RR型
		if (balance < -1 && getBalanceFactor(node->right) <= 0) {
			return leftRotate(std::move(node));
		}

		// 情况3：LR型
		if (balance > 1 && getBalanceFactor(node->left) < 0) {
			node->left = leftRotate(std::move(node->left));
			return rightRotate(std::move(node));
		}

		// 情况4：RL型
		if (balance < -1 && getBalanceFactor(node->right) > 0) {
			node->right = rightRotate(std::move(node->right));
			return leftRotate(std::move(node));
		}

		return node;
	}

	// 中序遍历（内部实现，升序输出）
	void inorderHelper(const std::shared_ptr<AVLNode<T>>& node, std::function<void(const T&)> callback) const {
		if (node) {
			inorderHelper(node->left, callback);
			callback(node->data);
			inorderHelper(node->right, callback);
		}
	}

public:
	// 构造函数
	AVLTree() : root_(nullptr), comp_() {}
	explicit AVLTree(const Comparator& comp) : root_(nullptr), comp_(comp) {}

	// 插入元素
	void insert(const T& val) {
		root_ = insertHelper(std::move(root_), val);
	}

	// 移动插入（优化右值性能）
	void insert(T&& val) {
		root_ = insertHelper(std::move(root_), std::move(val));
	}

	// 查找元素
	bool find(const T& val) const {
		return findHelper(root_, val);
	}

	// 按值查找对象
	const std::shared_ptr<AVLNode<T>> getNode(const T& val) const {
		return getNodeHelper(root_, val);
	}

	// 删除元素
	void erase(const T& val) {
		root_ = deleteHelper(std::move(root_), val);
	}

	// 中序遍历（默认升序，可通过自定义比较器改变顺序）
	void inorderTraversal(std::function<void(const T&)> callback) const {
		inorderHelper(root_, callback);
	}

	// 判断树是否为空
	bool empty() const {
		return !root_;
	}

	// 获取树的高度
	int getTreeHeight() const {
		return getHeight(root_);
	}
};