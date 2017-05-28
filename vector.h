#ifndef AISDI_LINEAR_VECTOR_H
#define AISDI_LINEAR_VECTOR_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>

namespace aisdi
{

template <typename Type>
class Vector
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

  Vector()
  {
    this->containerPtr = new Type[10];
    this->size = 0;
    this->maxSize = 10;
  }

  Vector(std::initializer_list<Type> l) : Vector()
  {
    for(auto i : l){
        append(i);
    }
  }

  Vector(const Vector& other) : Vector()
  {
    for(const_iterator i = other.begin(); i != other.end(); i++){
        append(*i);
    }
  }

  Vector(Vector&& other)
  {
    this->containerPtr = other.containerPtr;
    this->maxSize = other.maxSize;
    this->size = other.size;

    other.containerPtr = nullptr;
    other.maxSize = 0;
    other.size = 0;
  }

  ~Vector()
  {
      delete[] this->containerPtr;
  }

  Vector& operator=(const Vector& other)
  {
    erase(this->begin(), this->end());
    for(const_iterator it = other.cbegin(); it != other.cend(); it++) append(*it);
    return *this;
  }

  Vector& operator=(Vector&& other)
  {
    erase(this->begin(), this->end());
    this->containerPtr = other.containerPtr;
    this->size = other.size, this->maxSize = other.maxSize;
    other.containerPtr = nullptr;
    other.size = other.maxSize = 0;
    return *this;
  }

  const Type& operator[](size_type index) const{
    return containerPtr[index];
  }

  Type& operator[](size_type index){
    return containerPtr[index];
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
    if(this->size == this->maxSize){
        Type *newContainerPtr = new Type[this->maxSize*=2];
        for(size_type i = 0; i<this->size; i++) newContainerPtr[i] = this->containerPtr[i];
        delete[] this->containerPtr;
        this->containerPtr = newContainerPtr;
    }
    this->containerPtr[this->size] = item;
    this->size++;
  }

  void prepend(const Type& item)
  {
    if(this->size == this->maxSize){
        Type *newContainerPtr = new Type[this->maxSize*=2];
        for(size_type i = 0; i<this->size; i++) newContainerPtr[i+1] = this->containerPtr[i];
        delete []containerPtr;
        containerPtr = newContainerPtr;
    }
    else{
        for(size_type i = this->size; i>0; i--) containerPtr[i] = containerPtr[i-1];
    }

    this->containerPtr[0] = item;

    this->size++;
  }

  void insert(const const_iterator& insertPosition, const Type& item)
  {
    if(this->size == this->maxSize){
        Type *newContainerPtr = new Type[this->maxSize*=2];
        for(size_type i = 0; i<insertPosition.position; i++) newContainerPtr[i] = this->containerPtr[i];
        for(size_type i = insertPosition.position; i<this->size; i++) newContainerPtr[i+1] = this->containerPtr[i];
        delete[] this->containerPtr;
        this->containerPtr = newContainerPtr;
    }
    else for(size_type i = this->size; i > insertPosition.position; i--) this->containerPtr[i] = this->containerPtr[i-1];
    this->containerPtr[insertPosition.position] = item;
    this->size++;
  }

  Type popFirst()
  {
    if(isEmpty()) throw std::logic_error("Popping element when the vector is empty");

    Type value = *(this->begin());

    for(size_type i = 1; i < this->size; i++){
        this->containerPtr[i-1] = this->containerPtr[i];
    }
    this->size--;

    return value;
  }

  Type popLast()
  {
    if(isEmpty()) throw std::logic_error("Popping element when the vector is empty");
    return this->containerPtr[--this->size];
  }

  void erase(const const_iterator& possition)
  {
    if(isEmpty()) throw std::out_of_range("Erasing when array is empty");
    if(possition.position == this->size) throw std::out_of_range("Erasing end() element");

    size_type pos = possition.position;

    for(size_type i = pos; i < size; i++) this->containerPtr[i] = this->containerPtr[i+1];
    this->size--;
  }

  void erase(const const_iterator& firstIncluded, const const_iterator& lastExcluded)
  {
    if(isEmpty()) throw std::out_of_range("Erasing when array is empty");

    size_type pos1 = firstIncluded.position, pos2 = lastExcluded.position;

    for(size_type i = pos1, j = i+(pos2-pos1); j < this->size; i++, j++) this->containerPtr[i] = this->containerPtr[j];
    this->size -= (pos2-pos1);
  }

  iterator begin()
  {
    return iterator(*this, 0);
  }

  iterator end()
  {
    return iterator(*this, size);
  }

  const_iterator cbegin() const
  {
    return const_iterator(*this, 0);
  }

  const_iterator cend() const
  {
    return const_iterator(*this, size);
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
    pointer containerPtr;
    size_type maxSize, size;
};

template <typename Type>
class Vector<Type>::ConstIterator
{
protected:
  const Vector<Type> &vec;

public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename Vector::value_type;
  using difference_type = typename Vector::difference_type;
  using pointer = typename Vector::const_pointer;
  using reference = typename Vector::const_reference;

  size_t position;

  explicit ConstIterator(const Vector &v, size_t pos) : vec(v), position(pos) {}

  reference operator*() const
  {
    if(this->position == vec.getSize()) throw std::out_of_range("Tried to get value of end() element");
    return vec[position];
  }

  ConstIterator& operator++()
  {
    if(++(this->position) > vec.getSize()) throw std::out_of_range("Array exceeded");
    return *this;
  }

  ConstIterator operator++(int)
  {
    ConstIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  ConstIterator& operator--()
  {
    if(this->position == 0) throw std::out_of_range("Array exceeded");
    this->position--;
    return *this;
  }

  ConstIterator operator--(int)
  {
    ConstIterator tmp = *this;
    --(*this);
    return tmp;
  }

  ConstIterator operator+(difference_type d) const
  {
    ConstIterator tmp = *this;
    for(int i = 0; i < d; i++) tmp++;
    return tmp;
  }

  ConstIterator operator-(difference_type d) const
  {
    ConstIterator tmp = *this;
    for(int i = 0; i < d; i++) tmp--;
    return tmp;
  }

  bool operator==(const ConstIterator& other) const
  {
    return this->position == other.position;
  }

  bool operator!=(const ConstIterator& other) const
  {
    return !(*this == other);
  }


};

template <typename Type>
class Vector<Type>::Iterator : public Vector<Type>::ConstIterator
{
public:
  using pointer = typename Vector::pointer;
  using reference = typename Vector::reference;

  explicit Iterator()
  {}

  Iterator(const ConstIterator& other)
    : ConstIterator(other)
  {}

  explicit Iterator(Vector &v, size_t pos) : ConstIterator(v, pos) {}

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
#endif // AISDI_LINEAR_VECTOR_H
