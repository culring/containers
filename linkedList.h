#ifndef AISDI_LINEAR_LINKEDLIST_H
#define AISDI_LINEAR_LINKEDLIST_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>

template <typename Type>
struct Node{
    Type value;
    Node *next;
};

namespace aisdi
{

template <typename Type>
class LinkedList
{
public:
  using difference_type = std::ptrdiff_t;
  using size_type = std::size_t;
  using value_type = Type;
  using pointer = Type*;
  using reference = Type&;
  using const_pointer = const Type*;
  using const_reference = const Type&;

  class ConstIterator;
  class Iterator;
  using iterator = Iterator;
  using const_iterator = ConstIterator;

  LinkedList()
  {
    this->head = this->tail = nullptr;
    this->size = 0;
  }

  LinkedList(std::initializer_list<Type> l) : LinkedList()
  {
    for(auto i : l) append(i);
  }

  LinkedList(const LinkedList& other) : LinkedList()
  {
    for(iterator it = other.cbegin(); it != other.cend(); it++) append(*it);
  }

  LinkedList(LinkedList&& other)
  {
    this->head = other.head;
    this->tail = other.tail;
    this->size = other.getSize();
    other.head = other.tail = nullptr;
    other.size = 0;
  }

  ~LinkedList()
  {
    wipe();
  }

  LinkedList& operator=(const LinkedList& other)
  {
    wipe();

    for(ConstIterator it = other.cbegin(); it != other.cend(); it++) append(*it);

    return *this;
  }

  LinkedList& operator=(LinkedList&& other)
  {
    wipe();
    this->head = other.head;
    this->tail = other.tail;
    this->size = other.size;
    other.head = other.tail = nullptr;
    other.size = 0;

    return *this;
  }

  bool isEmpty() const
  {
    return (!this->size);
  }

  size_type getSize() const
  {
    return this->size;
  }

  void append(const Type& item)
  {
    Node<Type> *node = new Node<Type>;
    node->value = item, node->next = nullptr;

    if(isEmpty()) this->head = this->tail = node;
    else{
        this->tail->next = node;
        this->tail = node;
    }

    this->size++;
  }

  void prepend(const Type& item)
  {
    Node<Type> *node = new Node<Type>;
    node->value = item, node->next = nullptr;

    if(isEmpty()) this->head = this->tail = node;
    else{
        node->next = this->head;
        this->head = node;
    }

    this->size++;
  }

  void insert(const const_iterator& insertPosition, const Type& item)
  {
    if(insertPosition.position > this->size) throw std::out_of_range("Exceeded a list while inserting an element");

    if(insertPosition.position == this->size){
        append(item);
        return;
    }

    if(insertPosition.position == 0){
        prepend(item);
        return;
    }

    Node<Type> *node = new Node<Type>;
    node->value = item, node->next = nullptr;

    Node<Type> *nodeBefore = getElementBefore(insertPosition);

    node->next = nodeBefore->next;
    nodeBefore->next = node;

    this->size++;
  }

  Type popFirst()
  {
    if(isEmpty()) throw std::logic_error("Popping element from list when it is empty");

    Type temp = this->head->value;
    Node<Type> *tempPtr = this->head;
    this->head = this->head->next;
    delete tempPtr;
    if(this->size == 1) this->tail = nullptr;
    this->size--;

    return temp;
  }

  Type popLast()
  {
    if(isEmpty()) throw std::logic_error("Popping an element from a list when it is empty");

    if(this->size == 1){
        Type temp = this->head->value;
        delete this->head;
        this->head = this->tail = nullptr;
        this->size = 0;

        return temp;
    }

    Node<Type> *temp = getElementBefore(cend()-1);
    Type tempValue = this->tail->value;
    delete this->tail;
    temp->next = nullptr;
    this->tail = temp;
    this->size--;

    return tempValue;
  }

  void erase(const const_iterator& possition)
  {
    if(isEmpty()) throw std::out_of_range("Erasing an element from a list when it is empty");
    if(possition == end()) throw std::out_of_range("Erasing end() element");

    Node<Type> *nodeBefore = getElementBefore(possition);

    if(nodeBefore == nullptr){
        Node<Type> *temp = this->head->next;
        delete this->head;
        this->head = temp;
    }
    else{
        if(possition.position == this->size-1) this->tail = nodeBefore;
        Node<Type> *temp = nodeBefore->next->next;
        delete nodeBefore->next;
        nodeBefore->next = temp;
    }

    this->size--;
  }

  void erase(const const_iterator& firstIncluded, const const_iterator& lastExcluded)
  {
    if(lastExcluded.position > this->size) throw std::out_of_range("Exceeding a list while erasing");

    for(size_type i = 0; i < lastExcluded.position - firstIncluded.position; i++) erase(firstIncluded);
  }

  void wipe()
  {
    while(this->size) erase(begin());
  }

  iterator begin()
  {
    return iterator(*this, 0, head);
  }

  iterator end()
  {
    return iterator(*this, this->size, head);
  }

  const_iterator cbegin() const
  {
    return const_iterator(*this, 0, head);
  }

  const_iterator cend() const
  {
    return const_iterator(*this, this->size, head);
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
    Node<Type> *head, *tail;
    size_type size;

    Node<Type>* getElementBefore(const const_iterator& possition){
        if(possition.position > this->size) throw std::out_of_range("Getting element beyond a list");

        if(possition.position == 0) return nullptr;

        Node<Type> *temp = this->head;
        size_type i = 0;
        while(i < possition.position-1) temp = temp->next, i++;
        return temp;
    }
};

template <typename Type>
class LinkedList<Type>::ConstIterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename LinkedList::value_type;
  using difference_type = typename LinkedList::difference_type;
  using pointer = typename LinkedList::const_pointer;
  using reference = typename LinkedList::const_reference;

  explicit ConstIterator(const LinkedList<Type> &l, size_type pos, Node<Type> *h) : head(h), list(l), position(pos)  {}

  reference operator*() const
  {
    if(this->position >= list.getSize()) throw std::out_of_range("List exceeded when dereferencing");

    Node<Type> *temp = this->head;
    size_type i = 0;
    while(i < this->position) temp = temp->next, i++;
    return temp->value;
  }

  ConstIterator& operator++()
  {
    this->position++;
    if(list.getSize()+1 == this->position) throw std::out_of_range("List exceeded");

    return *this;
  }

  ConstIterator operator++(int)
  {
    ConstIterator temp = *this;
    ++(*this);

    return temp;
  }

  ConstIterator& operator--()
  {
    if(!this->position) throw std::out_of_range("List exceeded");

    this->position--;

    return *this;
  }

  ConstIterator operator--(int)
  {
    ConstIterator temp = *this;
    --(*this);

    return temp;
  }

  ConstIterator operator+(difference_type d) const
  {
    ConstIterator it = *this;
    for(difference_type i = 0; i < d; i++) it++;

    return it;
  }

  ConstIterator operator-(difference_type d) const
  {
    ConstIterator it = *this;
    for(difference_type i = 0; i < d; i++) it--;

    return it;
  }

  bool operator==(const ConstIterator& other) const
  {
    return (this->head == other.head && this->position == other.position);
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }

protected:
    Node<Type> *head;
    const LinkedList<Type> &list;

public:
    size_type position;
};

template <typename Type>
class LinkedList<Type>::Iterator : public LinkedList<Type>::ConstIterator
{
public:
  using pointer = typename LinkedList::pointer;
  using reference = typename LinkedList::reference;

  explicit Iterator(const LinkedList<Type> &l, size_type pos, Node<Type> *h) : ConstIterator(l, pos, h) {}

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

  Iterator operator+(difference_type d) const
  {
    return ConstIterator::operator+(d);
  }

  Iterator operator-(difference_type d) const
  {
    return ConstIterator::operator-(d);
  }

  reference operator*() const
  {
    // ugly cast, yet reduces code duplication.
    return const_cast<reference>(ConstIterator::operator*());
  }
};

}

#endif // AISDI_LINEAR_LINKEDLIST_H
