#ifndef AISDI_MAPS_TREEMAP_H
#define AISDI_MAPS_TREEMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iostream>

namespace aisdi
{
template <typename KeyType, typename ValueType>
class TreeMap
{
// private:
    struct Node;

public:
  using key_type = KeyType;
  using mapped_type = ValueType;
  using value_type = std::pair<const key_type, mapped_type>;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;

  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;

  TreeMap() : _root(nullptr), _maxNode(nullptr), _minNode(nullptr), _size(0) {}

  TreeMap(std::initializer_list<value_type> list) : TreeMap()
  {
    for(auto i : list) (*this)[i.first] = i.second;
  }

  TreeMap(const TreeMap& other) : TreeMap()
  {
    //copying
    _root = duplicateTree(nullptr, other._root, other._minNode, other._maxNode);
    _size = other._size;
  }
  
  //creates the same tree hierarchy as in the 'other' tree
  //works recursively and returns the node it creates
  Node* duplicateTree(Node *source, Node *other, Node *otherMinNode, Node *otherMaxNode){
    if(other == nullptr) return nullptr;
    
    Node *newNode = new Node(other->value);
    if(other == otherMinNode) _minNode = newNode;
    if(other == otherMaxNode) _maxNode = newNode;
    newNode->parent = source;
    newNode->left = duplicateTree(newNode, other->left, otherMinNode, otherMaxNode);
    newNode->right = duplicateTree(newNode, other->right, otherMinNode, otherMaxNode);
    return newNode;
  }

  TreeMap(TreeMap&& other)
  {
    _root = other._root;
    _minNode = other._minNode;
    _maxNode = other._maxNode;

    other._root = other._minNode = other._maxNode = nullptr;

    _size = other._size;
    other._size = 0;
  }

  ~TreeMap(){ clear(); }

  TreeMap& operator=(const TreeMap& other)
  {
    if(&other == this) return *this;
    
    clear();

    _root = duplicateTree(nullptr, other._root, other._minNode, other._maxNode);
    _size = other._size;

    return *this;
  }

  TreeMap& operator=(TreeMap&& other)
  {
    if(&other == this) return *this;

    clear();

    _root = other._root;
    _minNode = other._minNode;
    _maxNode = other._maxNode;

    other._root = other._minNode = other._maxNode = nullptr;

    _size = other._size;
    other._size = 0;

    return *this;
  }

  bool isEmpty() const
  {
    return !_size;
  }

  mapped_type& operator[](const key_type& key)
  {
    iterator it = find(key);
    //if element of the given key is in the tree
    if(it != end()) return it->second;

    mapped_type pom;
    Node *newNode = new Node(std::make_pair(key, pom));

    //updating minimum and maximum node
    if(!_size || _minNode->first > key) _minNode = newNode;
    if(!_size || _maxNode->first < key) _maxNode = newNode;

    //if the tree is empty
    if(isEmpty()){
        _root = newNode;
        ++_size;
        return _root->second;
    }

    //if the tree is not empty
    Node *ptr = _root;
    while(true){
        if(key < ptr->first){
            if(ptr->left != nullptr) ptr = ptr->left;
            else{
                ptr->left = newNode;
                break;
            }
        }
        else{
            if(ptr->right != nullptr) ptr = ptr->right;
            else{
                ptr->right = newNode;
                break;
            }
        }
    }
    newNode->parent = ptr;
    ++_size;

    return newNode->second;
  }

  const mapped_type& valueOf(const key_type& key) const
  {
    const_iterator c_it = find(key);
    if( c_it == cend() ) throw std::out_of_range("ValueOf didn't find the element");
    return c_it->second;
  }

  mapped_type& valueOf(const key_type& key)
  {
    iterator it = find(key);
    if( it == end() ) throw std::out_of_range("ValueOf didn't find the element");
    return it->second;
  }

  const_iterator find(const key_type& key) const
  {
    const_iterator c_it = cbegin();
    while(c_it != cend() && key != c_it->first) ++c_it;
    return c_it;
  }

  iterator find(const key_type& key)
  {
    iterator it = begin();
    while(it != end() && key != it->first) ++it;
    return it;
  }

  void remove(const key_type& key)
  {
    const_iterator c_it = find(key);
    if( c_it == end() ) throw std::out_of_range("Remove didn't find the element");

    --_size;

    Node *toRemove = search(c_it), *pom = toRemove;
    
    //updating _maxNode and _minNode
    if(toRemove == _maxNode){
        if(!_size) _maxNode = nullptr;
        else _maxNode = search(--c_it);
    }
    if(toRemove == _minNode){
        if(!_size) _minNode = nullptr;
        else _minNode = search(++c_it);
    }

    //searching for node to replace the removing node
    //the greatest value in the left subtree
    if(pom->left != nullptr){
        pom = pom->left;
        while(pom->right != nullptr) pom = pom->right; 

        //inserting pom into toRemove's place
        if(pom->parent != toRemove){
            pom->parent->right = pom->left;
            pom->left = toRemove->left;
        }
        else toRemove->left = nullptr;
        pom->right = toRemove->right;
    }
    //the least value in the right subtree
    else if(pom->right != nullptr){
        pom = pom->right;
        while(pom->left != nullptr) pom = pom->left;

        //inserting pom into toRemove's place
        if(pom->parent != toRemove){
            pom->parent->left = pom->right;
            pom->right = toRemove->right;
        }
        else toRemove->right = nullptr;
        pom->left = toRemove->left;
    }
    //toRemove doesn't have neither left nor right node - it is a leaf!
    //so we can just delete the node without any consequences
    else{
        if(toRemove != _root){
            if(isLeftChild(toRemove)) toRemove->parent->left = nullptr;
            else toRemove->parent->right = nullptr;
        }
        else _root = nullptr;

        delete toRemove;
        return;
    }

    //switch pom's parent to toRemove's parent
    pom->parent = toRemove->parent;
    
    //if toRemove is not the root then inform its parent about changing children
    if(toRemove != _root){
        if(isLeftChild(toRemove)) toRemove->parent->left = pom;
        else toRemove->parent->right = pom;
    }
    else _root = pom;
    
    //inform children of toRemove about changing their parent
    if(toRemove->left != nullptr) toRemove->left->parent = pom;
    if(toRemove->right != nullptr) toRemove->right->parent = pom;

    delete toRemove;
  }

  void remove(const const_iterator& it)
  {
    remove(it->first);
  }

  void clear(){
    erase(_root);
    _size = 0;
    _root = _minNode = _maxNode = nullptr;
  }
  
  void erase(Node *v){
    if(v == nullptr) return;
    erase(v->left);
    erase(v->right);
    delete v;
  }

  size_type getSize() const
  {
    return _size;
  }
  
  void display() const
  {
    for(auto it = cbegin(); it != cend(); ++it) std::cout << it->first << " " << it->second << std::endl;
  }

  bool operator==(const TreeMap& other) const
  {
    if(_size != other._size) return false;
    for(const_iterator it1 = cbegin(), it2 = other.cbegin(); it1 != cend(); ++it1, ++it2)
        if( (it1->first != it2->first) || (it1->second != it2->second) ) return false;
    return true;
  }

  bool operator!=(const TreeMap& other) const
  {
    return !(*this == other);
  }

  iterator begin()
  {
    if(_size) return iterator(_minNode, false);
    else return iterator(_minNode, true);
  }

  iterator end()
  {
    return iterator(_maxNode, true);
  }

  const_iterator cbegin() const
  {
    if(_size) return const_iterator(_minNode, false);
    else return const_iterator(_minNode, true);
  }

  const_iterator cend() const
  {
    return const_iterator(_maxNode, true);
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }

private:
  struct Node{
    value_type value;
    const key_type &first;
    mapped_type &second;
    Node *left, *right, *parent;

    Node(const value_type &v) : value(v), first(value.first), second(value.second), left(nullptr), right(nullptr), parent(nullptr) {}
  };
  Node *_root, *_maxNode, *_minNode;
  size_type _size;

  //returns nullptr if node with indicated key not found
  Node* search(const const_iterator c_it) const {
    if(c_it == cend()) throw std::out_of_range("Search can't find the element");
    const key_type key = c_it->first;
    Node *pom = _root;
    while(pom != nullptr && pom->first != key){
        if(key < pom->first) pom = pom->left;
        else pom = pom->right;
    }
    return pom;
  }

  bool isLeftChild(Node *node){
    return (node == node->parent->left);
  }
};

template <typename KeyType, typename ValueType>
class TreeMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename TreeMap::const_reference;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename TreeMap::value_type;
  using pointer = const typename TreeMap::value_type*;

  explicit ConstIterator()
  {
    _position = nullptr;
    _isEnd = true;
  }

  ConstIterator(Node *pos, bool isEnd) : _position(pos), _isEnd(isEnd) {}

  ConstIterator(const ConstIterator& other)
  {
    _position = other._position;
    _isEnd = other._isEnd;
  }

  ConstIterator& operator++()
  {
    if(_isEnd) throw std::out_of_range("Tried to iterate beyond the tree");

    //we found greater value in right subtree
    if(_position->right != nullptr){
        _position = _position->right;
        while(_position->left != nullptr) _position = _position->left;
        return *this;
    }

    //there is maybe some greater value up in the tree
    Node *pom = _position, *prev = _position;
    while((pom = pom->parent) != nullptr){
        if(isLeftChild(prev)){
            _position = pom;
            break;
        }
        prev = pom;
    }
    if(pom == nullptr) _isEnd = true;

    return *this;
  }

  ConstIterator operator++(int)
  {
    return (++(*this));
  }

  ConstIterator& operator--()
  {
    //if the tree is empty
    if(_position == nullptr) throw std::out_of_range("Tried to iterate beyond the tree");

    //no longer being end()
    if(_isEnd){
        _isEnd = false;
        return *this;
    }

    //we found smaller value in left subtree
    if(_position->left != nullptr){
        _position = _position->left;
        while(_position->right != nullptr) _position = _position->right;
        return *this;
    }

    //there is maybe some smaller value up in the tree
    Node *pom = _position, *prev = _position;
    while((pom = pom->parent) != nullptr){
        //if we come from right subtree
        if(!isLeftChild(prev)){
            _position = pom;
            return *this;
        }
        prev = pom;
    }
    if(pom == nullptr) throw std::out_of_range("Tried to iterate beyond the tree");

    return *this;
  }

  ConstIterator operator--(int)
  {
    return (--(*this));
  }

  reference operator*() const
  {
    if(_isEnd) throw std::out_of_range("Tried to get the value of the end()");
    return _position->value;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  bool operator==(const ConstIterator& other) const
  {
    return (_position == other._position && _isEnd == other._isEnd);
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }

protected:
  Node *_position;
  bool _isEnd;

  bool isLeftChild(Node *node){
    return (node == node->parent->left);
  }
};

template <typename KeyType, typename ValueType>
class TreeMap<KeyType, ValueType>::Iterator : public TreeMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename TreeMap::reference;
  using pointer = typename TreeMap::value_type*;

  explicit Iterator() : ConstIterator()
  {}

  Iterator(const ConstIterator& other)
    : ConstIterator(other)
  {}

  Iterator(Node *pos, bool isEnd) : ConstIterator(pos, isEnd) { }

  Iterator& operator++()
  {
    ConstIterator::operator++();
    return *this;
  }

  Iterator operator++(int)
  {
    auto result = *this;
    ConstIterator::operator++();
    return result;
  }

  Iterator& operator--()
  {
    ConstIterator::operator--();
    return *this;
  }

  Iterator operator--(int)
  {
    auto result = *this;
    ConstIterator::operator--();
    return result;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  reference operator*() const
  {
    // ugly cast, yet reduces code duplication.
    return const_cast<reference>(ConstIterator::operator*());
  }
};

}

#endif /* AISDI_MAPS_MAP_H */
