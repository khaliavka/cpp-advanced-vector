#pragma once
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

template <typename T>
class RawMemory {
 public:
  RawMemory() = default;
  explicit RawMemory(size_t capacity);

  RawMemory(const RawMemory&) = delete;
  RawMemory& operator=(const RawMemory&) = delete;

  RawMemory(RawMemory&& other) noexcept;
  RawMemory& operator=(RawMemory&& rhs) noexcept;

  ~RawMemory();

  T* operator+(size_t offset) noexcept;
  const T* operator+(size_t offset) const noexcept;
  const T& operator[](size_t index) const noexcept;
  T& operator[](size_t index) noexcept;
  void Swap(RawMemory& other) noexcept;
  const T* GetAddress() const noexcept;
  T* GetAddress() noexcept;
  size_t Capacity() const;

 private:
  static T* Allocate(size_t n);
  static void Deallocate(T* buf) noexcept;

  T* buffer_ = nullptr;
  size_t capacity_ = 0;
};

template <typename T>
class Vector {
 public:
  using iterator = T*;
  using const_iterator = const T*;

  Vector() = default;
  explicit Vector(size_t size);
  Vector(const Vector& other);
  Vector(Vector&& other) noexcept;
  Vector& operator=(const Vector& rhs);
  Vector& operator=(Vector&& rhs) noexcept;
  ~Vector();

  size_t Size() const noexcept;
  size_t Capacity() const noexcept;
  T& operator[](size_t index) noexcept;
  const T& operator[](size_t index) const noexcept;
  void Reserve(size_t new_capacity);
  void Resize(size_t new_size);
  iterator Insert(const_iterator pos, const T& value);
  iterator Insert(const_iterator pos, T&& value);
  template <typename... Args>
  iterator Emplace(const_iterator pos, Args&&... args);
  iterator Erase(const_iterator pos);
  void PushBack(const T& value);
  void PushBack(T&& value);
  template <typename... Args>
  T& EmplaceBack(Args&&... args);
  void PopBack();
  T& Back() noexcept;
  void Swap(Vector& other) noexcept;

  iterator begin() noexcept;
  iterator end() noexcept;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

 private:
  template <typename InputIt, typename OutputIt>
  void MoveOrCopy(InputIt first, InputIt last, OutputIt d_first);
  template <typename InputIt, typename OutputIt>
  void UninitMoveOrCopy(InputIt first, InputIt last, OutputIt d_first);
  template <typename InputIt, typename OutputIt>
  void TryUninitMoveOrCopy(InputIt first, InputIt last, OutputIt d_first,
                           OutputIt dy_first, OutputIt dy_last);
  template <typename InOutIt>
  void MoveOrCopyBackward(InOutIt first, InOutIt last);

  RawMemory<T> data_;
  size_t size_ = 0;
};

template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity)), capacity_(capacity) {}

template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept {
  Swap(other);
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept {
  if (this != &rhs) {
    Swap(rhs);
  }
  return *this;
}

template <typename T>
RawMemory<T>::~RawMemory() {
  Deallocate(buffer_);
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept {
  // Разрешается получать адрес ячейки памяти, следующей за последним
  // элементом массива
  assert(offset <= capacity_);
  return buffer_ + offset;
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept {
  return const_cast<RawMemory&>(*this) + offset;
}

template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept {
  return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept {
  assert(index < capacity_);
  return buffer_[index];
}

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept {
  std::swap(buffer_, other.buffer_);
  std::swap(capacity_, other.capacity_);
}

template <typename T>
const T* RawMemory<T>::GetAddress() const noexcept {
  return buffer_;
}

template <typename T>
T* RawMemory<T>::GetAddress() noexcept {
  return buffer_;
}

template <typename T>
size_t RawMemory<T>::Capacity() const {
  return capacity_;
}

template <typename T>
T* RawMemory<T>::Allocate(size_t n) {
  return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template <typename T>
void RawMemory<T>::Deallocate(T* buf) noexcept {
  operator delete(buf);
}

template <typename T>
Vector<T>::Vector(size_t size) : data_{size}, size_{size} {
  std::uninitialized_value_construct(begin(), end());
}

template <typename T>
Vector<T>::Vector(const Vector& other)
    : data_{other.size_}, size_{other.size_} {
  std::uninitialized_copy(other.begin(), other.end(), begin());
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept : data_{std::move(other.data_)} {
  std::swap(size_, other.size_);
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& rhs) {
  if (this == &rhs) {
    return *this;
  }
  if (rhs.size_ > data_.Capacity()) {
    Vector rhs_copy(rhs);
    Swap(rhs_copy);
  } else {
    if (rhs.size_ < size_) {
      std::copy(rhs.cbegin(), rhs.cend(), begin());
      std::destroy_n(begin() + rhs.size_, size_ - rhs.size_);
    } else {
      std::copy(rhs.cbegin(), rhs.cbegin() + size_, begin());
      std::uninitialized_copy_n(rhs.cbegin() + size_, rhs.size_ - size_, end());
    }
    size_ = rhs.size_;
  }
  return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector<T>&& rhs) noexcept {
  if (this != &rhs) {
    Swap(rhs);
  }
  return *this;
}

template <typename T>
Vector<T>::~Vector() {
  std::destroy(begin(), end());
}

template <typename T>
size_t Vector<T>::Size() const noexcept {
  return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
  return data_.Capacity();
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
  return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
  assert(index < size_);
  return data_[index];
}

template <typename T>
void Vector<T>::Reserve(size_t new_capacity) {
  if (new_capacity <= data_.Capacity()) {
    return;
  }
  RawMemory<T> new_data{new_capacity};
  UninitMoveOrCopy(begin(), end(), new_data.GetAddress());
  std::destroy(begin(), end());
  data_.Swap(new_data);
}

template <typename T>
void Vector<T>::Resize(size_t new_size) {
  if (new_size > size_) {
    Reserve(new_size);
    std::uninitialized_value_construct_n(end(), new_size - size_);
  } else {
    std::destroy_n(begin() + new_size, size_ - new_size);
  }
  size_ = new_size;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos,
                                               const T& value) {
  return Emplace(pos, value);
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, T&& value) {
  return Emplace(pos, std::move(value));
}

template <typename T>
template <typename... Args>
typename Vector<T>::iterator Vector<T>::Emplace(const_iterator pos,
                                                Args&&... args) {
  assert(pos >= begin() && pos <= end());
  auto pos_non_const = const_cast<iterator>(pos);
  if (size_ == data_.Capacity()) {
    RawMemory<T> new_data{size_ == 0 ? 1 : size_ * 2};
    auto distance_from_begin = pos - data_.GetAddress();
    auto new_begin = new_data.GetAddress();
    auto new_pos = new (new_data.GetAddress() + distance_from_begin)
        T(std::forward<Args>(args)...);
    TryUninitMoveOrCopy(begin(), pos_non_const, new_begin, new_pos,
                        new_pos + 1);
    TryUninitMoveOrCopy(pos_non_const, end(), new_pos + 1, new_begin,
                        new_pos + 1);
    std::destroy(begin(), end());
    data_.Swap(new_data);
    ++size_;
    return new_pos;
  }
  if (pos != end()) {
    T element(std::forward<Args>(args)...);
    MoveOrCopyBackward(pos_non_const, end());
    *pos_non_const = std::move(element);
  } else {
    new (end()) T(std::forward<Args>(args)...);
  }
  ++size_;
  return pos_non_const;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Erase(const_iterator pos) {
  assert(pos >= begin() && pos < end());
  auto pos_non_const = const_cast<iterator>(pos);
  MoveOrCopy(pos_non_const + 1, end(), pos_non_const);
    std::destroy_at(end() - 1);
  --size_;
  return pos_non_const;
}

template <typename T>
void Vector<T>::PushBack(const T& value) {
  EmplaceBack(value);
}

template <typename T>
void Vector<T>::PushBack(T&& value) {
  EmplaceBack(std::move(value));
}

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
  if (size_ == data_.Capacity()) {
    RawMemory<T> new_data{size_ == 0 ? 1 : size_ * 2};
    new (new_data.GetAddress() + size_) T(std::forward<Args>(args)...);
    UninitMoveOrCopy(begin(), end(), new_data.GetAddress());
    std::destroy(begin(), end());
    data_.Swap(new_data);
  } else {
    new (end()) T(std::forward<Args>(args)...);
  }
  ++size_;
  return Back();
}

template <typename T>
void Vector<T>::PopBack() {
  assert(size_ != 0);
  --size_;
  std::destroy_at(end());
}

template <typename T>
T& Vector<T>::Back() noexcept {
  return *(end() - 1);
}

template <typename T>
void Vector<T>::Swap(Vector& other) noexcept {
  data_.Swap(other.data_);
  std::swap(size_, other.size_);
}

template <typename T>
template <typename InputIt, typename OutputIt>
void Vector<T>::MoveOrCopy(InputIt first, InputIt last, OutputIt d_first) {
  if constexpr (std::is_nothrow_move_constructible_v<T> ||
                !std::is_copy_constructible_v<T>) {
    std::move(first, last, d_first);
  } else {
    std::copy(first, last, d_first);
  }
}

template <typename T>
template <typename InputIt, typename OutputIt>
void Vector<T>::UninitMoveOrCopy(InputIt first, InputIt last,
                                 OutputIt d_first) {
  if constexpr (std::is_nothrow_move_constructible_v<T> ||
                !std::is_copy_constructible_v<T>) {
    std::uninitialized_move(first, last, d_first);
  } else {
    std::uninitialized_copy(first, last, d_first);
  }
}

template <typename T>
template <typename InputIt, typename OutputIt>
void Vector<T>::TryUninitMoveOrCopy(InputIt first, InputIt last,
                                    OutputIt d_first, OutputIt dy_first,
                                    OutputIt dy_last) {
  try {
    UninitMoveOrCopy(first, last, d_first);
  } catch (...) {
    std::destroy(dy_first, dy_last);
    throw;
  }
}

template <typename T>
template <typename InOutIt>
void Vector<T>::MoveOrCopyBackward(InOutIt first, InOutIt last) {
  if constexpr (std::is_nothrow_move_constructible_v<T> ||
                std::is_nothrow_move_assignable_v<T> ||
                !std::is_copy_constructible_v<T> ||
                !std::is_copy_assignable_v<T>) {
    std::uninitialized_move(last - 1, last, last);
    std::move_backward(first, last - 1, last);
  } else {
    std::uninitialized_copy(last - 1, last, last);
    std::copy_backward(first, last - 1, last);
  }
}

template <typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept {
  return data_.GetAddress();
}

template <typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept {
  return data_.GetAddress() + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept {
  return const_cast<Vector<T>&>(*this).begin();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept {
  return const_cast<Vector<T>&>(*this).end();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept {
  return begin();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept {
  return end();
}
