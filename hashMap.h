#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <list>
#include <functional>
#include <typeinfo>

namespace aisdi
{

// template <typename KeyType, typename ValueType>


template <typename KeyType, typename ValueType>
class HashMap
{
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

  HashMap() : _size(0) { _hashArray = new std::list<node_ptr>[N]; }

  HashMap(std::initializer_list<value_type> list) : HashMap()
  {
    for(auto i : list)
        (*this)[i.first] = i.second;
  }

  HashMap(const HashMap& other) : HashMap() {
    for(auto it = other.begin(); it != other.end(); ++it)
        (*this)[it->first] = it->second;
  }

  HashMap(HashMap&& other)
  {
    _hashArray = other._hashArray;
    _size = other._size;
    other._hashArray = nullptr;
    other._size = 0;
  }

  HashMap& operator=(const HashMap& other)
  {
    if(this == &other) return *this;

    _size = 0;

    for(auto it = other.begin(); it != other.end(); ++it)
        (*this)[it->first] = it->second;

    return *this;
  }

  HashMap& operator=(HashMap&& other)
  {
    _hashArray = other._hashArray;
    _size = other._size;
    other._hashArray = nullptr;
    other._size = 0;

    return *this;
  }

  bool isEmpty() const
  {
    return !(_size);
  }

  mapped_type& operator[](const key_type& key)
  {
    size_type hash = getHash(key);

    //if there is an element in hashmap with given key
    iterator it = find(key);
    if(it != end()) return it->second;

    //we have to create new element
    mapped_type pom;
    Node *newNode = new Node(std::make_pair(key, pom));
    _hashArray[hash].push_back(newNode);

    ++_size;

    return newNode->second;
  }

  const mapped_type& valueOf(const key_type& key) const
  {
    const_iterator it = find(key);
    if(it != end()) return it->second;
    else throw std::out_of_range("Tried to get value of non-existing element");
  }

  mapped_type& valueOf(const key_type& key)
  {
    iterator it = find(key);
    if(it != end()) return it->second;
    else throw std::out_of_range("Tried to get value of non-existing element");
  }

  const_iterator find(const key_type& key) const
  {
    const_iterator it(key, _hashArray);
    return it;
  }

  iterator find(const key_type& key)
  {
    iterator it(key, _hashArray);
    return it;
  }

  void remove(const key_type& key)
  {
    list_it it;

    //we did not found such key in the the list
    if(getListIteratorByKey(key, it)) throw std::out_of_range("Tried to remove object that did not exist");

    --_size;
    size_type hash = getHash(key);
    _hashArray[hash].erase(it);
  }

  void remove(const const_iterator& it)
  {
    if(it == end()) throw std::out_of_range("Tried to remove end() element");
    remove(it->first);
  }

  size_type getSize() const
  {
    return _size;
  }

  bool operator==(const HashMap& other) const
  {
    if(_size != other._size) return 0;
    for(auto i = begin(), j = other.begin(); i != end(); ++i, ++j)
        if(*i != *j) return 0;
    return 1;
  }

  bool operator!=(const HashMap& other) const
  {
    return !(*this == other);
  }

  iterator begin()
  {
    return iterator(_hashArray, 0);
  }

  iterator end()
  {
    return iterator(_hashArray, N-1);
  }

  const_iterator cbegin() const
  {
    return const_iterator(_hashArray, 0);
  }

  const_iterator cend() const
  {
    return const_iterator(_hashArray, N-1);
  }

  const_iterator begin() const
  {
    return cbegin();
  }

  const_iterator end() const
  {
    return cend();
  }

protected:
    //size of hashmap array
    static const size_t N = 10;

    struct Node{
        value_type value;
        const key_type &first;
        mapped_type &second;

        Node(const value_type &v) : value(v), first(value.first), second(value.second) {}
    };
    typedef Node* node_ptr;
    typedef typename std::list<node_ptr>::iterator list_it;
    typedef typename std::list<node_ptr>::const_iterator const_list_it;

    std::list<node_ptr> *_hashArray;
    size_type _size;

    static size_type getHash(const key_type &key) {
        size_type hash = std::hash<key_type>{}(key);
        hash %= N;
        return hash;
    }

    //returns 1 if an element was not found in the list
    //otherwise returns 0
    //also puts an iterator of found element into
    //a passed iterator argument "it"
    bool getListIteratorByKey(const key_type &key, list_it &it){
        size_type hash = getHash(key);
        it = _hashArray[hash].begin();
        while(it != _hashArray[hash].end() && (*it)->first != key) ++it;
        if(it == _hashArray[hash].end()) return 1;
        return 0;
    }
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::const_reference;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename HashMap::value_type;
  using pointer = const typename HashMap::value_type*;

  explicit ConstIterator() : _hashArray(nullptr), _index(0), _it(nullptr) {}

  explicit ConstIterator(const std::list<node_ptr> *hashArray, size_type index) : _hashArray(hashArray), _index(index){
    //begin() iterator was called
    //we have to set _it for an iterator of a first non-empty list from _hashArray
    //or (when the map is empty) set it to end() of map
    if(!_index){
        //searching for first element in hashmap
        while(_index < N)
            if(!_hashArray[_index++].empty()) break;

        _it = _hashArray[--_index].cbegin();
    }
    //end() iterator was called
    else _it = _hashArray[index].cend();
  }

  ConstIterator(const KeyType &key, const std::list<node_ptr> *hashArray){
    _hashArray = hashArray;
    _index = getHash(key);
    _it = _hashArray[_index].cbegin();
    while(_it != _hashArray[_index].cend() && (*_it)->first != key) ++_it;
    if(_it == _hashArray[_index].cend()) _it = _hashArray[N-1].cend();
  }
  
  ConstIterator(const ConstIterator& other) : _hashArray(other._hashArray), _index(other._index), _it(other._it) {}

  ConstIterator& operator++()
  {
    if(_index == N-1){
        if(_it == _hashArray[N-1].cend())
            throw std::out_of_range("Tried to iterate beyond the hash map");
        else ++_it;
    }
    else{
        if(++_it == _hashArray[_index].cend()){
            while(++_index < N-1 && _hashArray[_index].empty());
            _it = _hashArray[_index].begin();
        }
    }
    return *this;
  }

  ConstIterator operator++(int)
  {
    return (++(*this));
  }

  ConstIterator& operator--()
  {
    if(_it == _hashArray[_index].begin())
    {
        while(--_index > 0 && _hashArray[_index].empty());
        if(_index == 0 && _hashArray[_index].empty()) throw std::out_of_range("Tried to iterate beyond the hash map");
        _it = --_hashArray[_index].end();
    }
    else --_it;

    return *this;
  }

  ConstIterator operator--(int)
  {
    return (--(*this));
  }

  reference operator*() const
  {
    if(_it == _hashArray[N-1].end()) throw std::out_of_range("Tried to dereference end() element");
    return (*_it)->value;
  }

  pointer operator->() const
  {
    return &this->operator*();
  }

  bool operator==(const ConstIterator& other) const
  {
    return (_hashArray == other._hashArray && _it == other._it);
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }

protected:
    const std::list<node_ptr> *_hashArray;
    size_type _index;
    const_list_it _it;
};

template <typename KeyType, typename ValueType>
class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
{
public:
  using reference = typename HashMap::reference;
  using pointer = typename HashMap::value_type*;

  explicit Iterator() : ConstIterator() {}

  Iterator(std::list<node_ptr> *hashArray, size_type index) : ConstIterator(hashArray, index) {}

  Iterator(const KeyType &key, const std::list<node_ptr> *hashArray) : ConstIterator(key, hashArray) {}
  
  Iterator(const ConstIterator& other)
    : ConstIterator(other)
  {}

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

private:
    list_it _it;
};

}

#endif /* AISDI_MAPS_HASHMAP_H */
