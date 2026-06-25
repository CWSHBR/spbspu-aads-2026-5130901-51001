#ifndef BSTREE_HPP
#define BSTREE_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace shaykhraziev
{
  template< class Key, class Value >
  struct BSTNode
  {
    const Key key;
    Value value;
    BSTNode* parent;
    BSTNode* left;
    BSTNode* right;
    bool fake;

    BSTNode():
      key(),
      value(),
      parent(nullptr),
      left(nullptr),
      right(nullptr),
      fake(true)
    {}

    BSTNode(Key&& newKey, Value&& newValue, BSTNode* fakeLeaf):
      key(std::move(newKey)),
      value(std::move(newValue)),
      parent(fakeLeaf),
      left(fakeLeaf),
      right(fakeLeaf),
      fake(false)
    {}
  };

  template< class Key, class Value, class Compare >
  class BSTIterator
  {
  public:
    using Node = BSTNode< Key, Value >;

    BSTIterator(Node* node = nullptr, Node* fakeLeaf = nullptr):
      node_(node),
      fakeLeaf_(fakeLeaf)
    {}

    Node& operator*() const
    {
      checkDereference();
      return *node_;
    }

    Node* operator->() const
    {
      checkDereference();
      return node_;
    }

    BSTIterator& operator++()
    {
      checkIterator();
      if (node_ == fakeLeaf_)
      {
        return *this;
      }
      if (node_->right != fakeLeaf_)
      {
        node_ = minimum(node_->right);
        return *this;
      }
      Node* parent = node_->parent;
      while (parent != fakeLeaf_ && node_ == parent->right)
      {
        node_ = parent;
        parent = parent->parent;
      }
      node_ = parent;
      return *this;
    }

    BSTIterator operator++(int)
    {
      BSTIterator tmp(*this);
      ++(*this);
      return tmp;
    }

    BSTIterator& operator--()
    {
      checkIterator();
      if (node_ == fakeLeaf_)
      {
        if (fakeLeaf_->left != fakeLeaf_)
        {
          node_ = maximum(fakeLeaf_->left);
        }
        return *this;
      }
      if (node_->left != fakeLeaf_)
      {
        node_ = maximum(node_->left);
        return *this;
      }
      Node* parent = node_->parent;
      while (parent != fakeLeaf_ && node_ == parent->left)
      {
        node_ = parent;
        parent = parent->parent;
      }
      node_ = parent;
      return *this;
    }

    BSTIterator operator--(int)
    {
      BSTIterator tmp(*this);
      --(*this);
      return tmp;
    }

    bool operator==(const BSTIterator& other) const
    {
      return node_ == other.node_;
    }

    bool operator!=(const BSTIterator& other) const
    {
      return !(*this == other);
    }

  private:
    Node* node_;
    Node* fakeLeaf_;

    static Node* minimum(Node* node) noexcept
    {
      while (!node->left->fake)
      {
        node = node->left;
      }
      return node;
    }

    static Node* maximum(Node* node) noexcept
    {
      while (!node->right->fake)
      {
        node = node->right;
      }
      return node;
    }

    void checkIterator() const
    {
      if (!node_ || !fakeLeaf_)
      {
        throw std::logic_error("invalid iterator");
      }
    }

    void checkDereference() const
    {
      checkIterator();
      if (node_ == fakeLeaf_)
      {
        throw std::logic_error("fake leaf dereference");
      }
    }

    template< class K, class V, class C >
    friend class BSTree;
    template< class K, class V, class C >
    friend class BSTConstIterator;
  };

  template< class Key, class Value, class Compare >
  class BSTConstIterator
  {
  public:
    using Node = BSTNode< Key, Value >;

    BSTConstIterator(const Node* node = nullptr, const Node* fakeLeaf = nullptr):
      node_(node),
      fakeLeaf_(fakeLeaf)
    {}

    BSTConstIterator(const BSTIterator< Key, Value, Compare >& other):
      node_(other.node_),
      fakeLeaf_(other.fakeLeaf_)
    {}

    const Node& operator*() const
    {
      checkDereference();
      return *node_;
    }

    const Node* operator->() const
    {
      checkDereference();
      return node_;
    }

    BSTConstIterator& operator++()
    {
      checkIterator();
      if (node_ == fakeLeaf_)
      {
        return *this;
      }
      if (node_->right != fakeLeaf_)
      {
        node_ = minimum(node_->right);
        return *this;
      }
      const Node* parent = node_->parent;
      while (parent != fakeLeaf_ && node_ == parent->right)
      {
        node_ = parent;
        parent = parent->parent;
      }
      node_ = parent;
      return *this;
    }

    BSTConstIterator operator++(int)
    {
      BSTConstIterator tmp(*this);
      ++(*this);
      return tmp;
    }

    BSTConstIterator& operator--()
    {
      checkIterator();
      if (node_ == fakeLeaf_)
      {
        if (fakeLeaf_->left != fakeLeaf_)
        {
          node_ = maximum(fakeLeaf_->left);
        }
        return *this;
      }
      if (node_->left != fakeLeaf_)
      {
        node_ = maximum(node_->left);
        return *this;
      }
      const Node* parent = node_->parent;
      while (parent != fakeLeaf_ && node_ == parent->left)
      {
        node_ = parent;
        parent = parent->parent;
      }
      node_ = parent;
      return *this;
    }

    BSTConstIterator operator--(int)
    {
      BSTConstIterator tmp(*this);
      --(*this);
      return tmp;
    }

    bool operator==(const BSTConstIterator& other) const
    {
      return node_ == other.node_;
    }

    bool operator!=(const BSTConstIterator& other) const
    {
      return !(*this == other);
    }

  private:
    const Node* node_;
    const Node* fakeLeaf_;

    static const Node* minimum(const Node* node) noexcept
    {
      while (!node->left->fake)
      {
        node = node->left;
      }
      return node;
    }

    static const Node* maximum(const Node* node) noexcept
    {
      while (!node->right->fake)
      {
        node = node->right;
      }
      return node;
    }

    void checkIterator() const
    {
      if (!node_ || !fakeLeaf_)
      {
        throw std::logic_error("invalid iterator");
      }
    }

    void checkDereference() const
    {
      checkIterator();
      if (node_ == fakeLeaf_)
      {
        throw std::logic_error("fake leaf dereference");
      }
    }

    template< class K, class V, class C >
    friend class BSTree;
  };

  template< class Key, class Value, class Compare >
  class BSTree
  {
  public:
    using Node = BSTNode< Key, Value >;
    using iterator = BSTIterator< Key, Value, Compare >;
    using const_iterator = BSTConstIterator< Key, Value, Compare >;

    BSTree():
      fakeLeaf_(new Node()),
      root_(fakeLeaf_),
      size_(0),
      compare_()
    {
      fakeLeaf_->left = fakeLeaf_;
      fakeLeaf_->right = fakeLeaf_;
    }

    ~BSTree()
    {
      clear();
      delete fakeLeaf_;
    }

    BSTree(const BSTree& other):
      BSTree()
    {
      copyFrom(other.root_, other.fakeLeaf_);
    }

    BSTree(BSTree&& other) noexcept:
      fakeLeaf_(other.fakeLeaf_),
      root_(other.root_),
      size_(other.size_),
      compare_(std::move(other.compare_))
    {
      other.fakeLeaf_ = new Node();
      other.fakeLeaf_->left = other.fakeLeaf_;
      other.fakeLeaf_->right = other.fakeLeaf_;
      other.root_ = other.fakeLeaf_;
      other.size_ = 0;
      refreshFakeLinks();
    }

    BSTree& operator=(const BSTree& other)
    {
      if (this != &other)
      {
        BSTree tmp(other);
        swap(tmp);
      }
      return *this;
    }

    BSTree& operator=(BSTree&& other) noexcept
    {
      if (this != &other)
      {
        clear();
        delete fakeLeaf_;
        fakeLeaf_ = other.fakeLeaf_;
        root_ = other.root_;
        size_ = other.size_;
        compare_ = std::move(other.compare_);
        other.fakeLeaf_ = new Node();
        other.fakeLeaf_->left = other.fakeLeaf_;
        other.fakeLeaf_->right = other.fakeLeaf_;
        other.root_ = other.fakeLeaf_;
        other.size_ = 0;
        refreshFakeLinks();
      }
      return *this;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    std::size_t size() const noexcept
    {
      return size_;
    }

    void clear() noexcept
    {
      clearNode(root_);
      root_ = fakeLeaf_;
      size_ = 0;
      fakeLeaf_->left = fakeLeaf_;
      fakeLeaf_->right = fakeLeaf_;
    }

    bool push(Key key, Value value)
    {
      Node* parent = fakeLeaf_;
      Node* current = root_;
      while (current != fakeLeaf_)
      {
        parent = current;
        if (compare_(key, current->key))
        {
          current = current->left;
        }
        else if (compare_(current->key, key))
        {
          current = current->right;
        }
        else
        {
          current->value = std::move(value);
          return false;
        }
      }

      Node* created = new Node(std::move(key), std::move(value), fakeLeaf_);
      created->parent = parent;
      if (parent == fakeLeaf_)
      {
        root_ = created;
      }
      else if (compare_(created->key, parent->key))
      {
        parent->left = created;
      }
      else
      {
        parent->right = created;
      }
      ++size_;
      refreshFakeLinks();
      return true;
    }

    bool has(const Key& key) const
    {
      return findNode(key) != fakeLeaf_;
    }

    Value& get(const Key& key)
    {
      Node* node = findNode(key);
      if (node == fakeLeaf_)
      {
        throw std::out_of_range("missing key");
      }
      return node->value;
    }

    const Value& get(const Key& key) const
    {
      const Node* node = findNode(key);
      if (node == fakeLeaf_)
      {
        throw std::out_of_range("missing key");
      }
      return node->value;
    }

    bool drop(const Key& key)
    {
      Node* node = findNode(key);
      if (node == fakeLeaf_)
      {
        return false;
      }

      if (node->left != fakeLeaf_ && node->right != fakeLeaf_)
      {
        Node* replacement = minimum(node->right);
        if (replacement->parent != node)
        {
          transplantNode(replacement, replacement->right);
          replacement->right = node->right;
          replacement->right->parent = replacement;
        }
        transplantNode(node, replacement);
        replacement->left = node->left;
        replacement->left->parent = replacement;
      }
      else if (node->left != fakeLeaf_)
      {
        transplantNode(node, node->left);
      }
      else
      {
        transplantNode(node, node->right);
      }

      delete node;
      --size_;
      refreshFakeLinks();
      return true;
    }

    iterator begin() noexcept
    {
      return iterator(empty() ? fakeLeaf_ : minimum(root_), fakeLeaf_);
    }

    iterator end() noexcept
    {
      return iterator(fakeLeaf_, fakeLeaf_);
    }

    const_iterator begin() const noexcept
    {
      return cbegin();
    }

    const_iterator end() const noexcept
    {
      return cend();
    }

    const_iterator cbegin() const noexcept
    {
      return const_iterator(empty() ? fakeLeaf_ : minimum(root_), fakeLeaf_);
    }

    const_iterator cend() const noexcept
    {
      return const_iterator(fakeLeaf_, fakeLeaf_);
    }

    std::size_t height() const noexcept
    {
      return heightNode(root_);
    }

    std::size_t height(const_iterator it) const
    {
      Node* node = mutableNode(it);
      if (node == fakeLeaf_)
      {
        return 0;
      }
      return heightNode(node);
    }

    iterator rotateLeft(const_iterator it)
    {
      Node* child = mutableNode(it);
      if (child == fakeLeaf_ || child->parent == fakeLeaf_ || child != child->parent->right)
      {
        throw std::logic_error("invalid left rotation");
      }
      Node* parent = child->parent;
      parent->right = child->left;
      if (child->left != fakeLeaf_)
      {
        child->left->parent = parent;
      }
      linkToGrandparent(parent, child);
      child->left = parent;
      parent->parent = child;
      refreshFakeLinks();
      return iterator(child, fakeLeaf_);
    }

    iterator rotateRight(const_iterator it)
    {
      Node* child = mutableNode(it);
      if (child == fakeLeaf_ || child->parent == fakeLeaf_ || child != child->parent->left)
      {
        throw std::logic_error("invalid right rotation");
      }
      Node* parent = child->parent;
      parent->left = child->right;
      if (child->right != fakeLeaf_)
      {
        child->right->parent = parent;
      }
      linkToGrandparent(parent, child);
      child->right = parent;
      parent->parent = child;
      refreshFakeLinks();
      return iterator(child, fakeLeaf_);
    }

    iterator rotateLargeLeft(const_iterator it)
    {
      Node* child = mutableNode(it);
      if (child == fakeLeaf_ || child->parent == fakeLeaf_ || child->parent->parent == fakeLeaf_ ||
          child != child->parent->left || child->parent != child->parent->parent->right)
      {
        throw std::logic_error("invalid large left rotation");
      }
      rotateRight(const_iterator(child, fakeLeaf_));
      return rotateLeft(const_iterator(child, fakeLeaf_));
    }

    iterator rotateLargeRight(const_iterator it)
    {
      Node* child = mutableNode(it);
      if (child == fakeLeaf_ || child->parent == fakeLeaf_ || child->parent->parent == fakeLeaf_ ||
          child != child->parent->right || child->parent != child->parent->parent->left)
      {
        throw std::logic_error("invalid large right rotation");
      }
      rotateLeft(const_iterator(child, fakeLeaf_));
      return rotateRight(const_iterator(child, fakeLeaf_));
    }

    void swap(BSTree& other) noexcept
    {
      Node* tmpFake = fakeLeaf_;
      fakeLeaf_ = other.fakeLeaf_;
      other.fakeLeaf_ = tmpFake;

      Node* tmpRoot = root_;
      root_ = other.root_;
      other.root_ = tmpRoot;

      std::size_t tmpSize = size_;
      size_ = other.size_;
      other.size_ = tmpSize;

      Compare tmpCompare = compare_;
      compare_ = other.compare_;
      other.compare_ = tmpCompare;

      refreshFakeLinks();
      other.refreshFakeLinks();
    }

  private:
    Node* fakeLeaf_;
    Node* root_;
    std::size_t size_;
    Compare compare_;

    Node* findNode(const Key& key) noexcept
    {
      Node* current = root_;
      while (current != fakeLeaf_)
      {
        if (compare_(key, current->key))
        {
          current = current->left;
        }
        else if (compare_(current->key, key))
        {
          current = current->right;
        }
        else
        {
          return current;
        }
      }
      return fakeLeaf_;
    }

    const Node* findNode(const Key& key) const noexcept
    {
      const Node* current = root_;
      while (current != fakeLeaf_)
      {
        if (compare_(key, current->key))
        {
          current = current->left;
        }
        else if (compare_(current->key, key))
        {
          current = current->right;
        }
        else
        {
          return current;
        }
      }
      return fakeLeaf_;
    }

    static Node* minimum(Node* node) noexcept
    {
      while (!node->left->fake)
      {
        node = node->left;
      }
      return node;
    }

    void clearNode(Node* node) noexcept
    {
      if (node == fakeLeaf_)
      {
        return;
      }
      clearNode(node->left);
      clearNode(node->right);
      delete node;
    }

    void copyFrom(const Node* node, const Node* otherFakeLeaf)
    {
      if (node == otherFakeLeaf)
      {
        return;
      }
      push(node->key, node->value);
      copyFrom(node->left, otherFakeLeaf);
      copyFrom(node->right, otherFakeLeaf);
    }

    std::size_t heightNode(const Node* node) const noexcept
    {
      if (node == fakeLeaf_)
      {
        return 0;
      }
      const std::size_t leftHeight = heightNode(node->left);
      const std::size_t rightHeight = heightNode(node->right);
      return (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
    }

    Node* mutableNode(const_iterator it) const
    {
      if (it.fakeLeaf_ != fakeLeaf_)
      {
        throw std::logic_error("foreign iterator");
      }
      return const_cast< Node* >(it.node_);
    }

    void transplantNode(Node* oldNode, Node* newNode) noexcept
    {
      if (oldNode->parent == fakeLeaf_)
      {
        root_ = newNode;
      }
      else if (oldNode == oldNode->parent->left)
      {
        oldNode->parent->left = newNode;
      }
      else
      {
        oldNode->parent->right = newNode;
      }
      if (newNode != fakeLeaf_)
      {
        newNode->parent = oldNode->parent;
      }
    }

    void linkToGrandparent(Node* oldChild, Node* newChild) noexcept
    {
      Node* grandparent = oldChild->parent;
      newChild->parent = grandparent;
      if (grandparent == fakeLeaf_)
      {
        root_ = newChild;
      }
      else if (oldChild == grandparent->left)
      {
        grandparent->left = newChild;
      }
      else
      {
        grandparent->right = newChild;
      }
    }

    void refreshFakeLinks() noexcept
    {
      fakeLeaf_->parent = fakeLeaf_;
      fakeLeaf_->left = root_;
      fakeLeaf_->right = fakeLeaf_;
      if (root_ != fakeLeaf_)
      {
        root_->parent = fakeLeaf_;
      }
    }
  };
}

#endif
