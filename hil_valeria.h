//
// Created by valeriahil on 23.06.18.
//

#ifndef CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_CIRCULAR_BUFFER_H

#include <iterator>

template<typename T>
struct circ_buff;

template <typename U>
struct Iterator;

template<typename T>
void swap(circ_buff<T> &a, circ_buff<T> &b) noexcept {
    std::swap(a.head_, b.head_);
    std::swap(a.tail_, b.tail_);
    std::swap(a.capacity, b.capacity);
    std::swap(a.buffer, b.buffer);
}

template<typename T>
struct circ_buff {
    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    friend Iterator<T>;
    friend void swap<T>(circ_buff& a, circ_buff& b) noexcept;

    circ_buff() {
        capacity = 2;
        buffer = static_cast<T *>(operator new[](sizeof(T) * capacity));
    }

    ~circ_buff() {
        delete[] buffer;
    }

    void clear() {
        tail_ = 0;
        head_ = 0;
    }

    circ_buff(circ_buff const &other) : circ_buff() {
        for (auto const &x : other) {
            push_back(x);
        }
    }

    circ_buff &operator=(circ_buff const &other) {
        circ_buff tmp(other);
        swap(*this, tmp);
        return *this;
    }

    bool empty() const {
        return head_ == tail_;
    }

    size_t size() const {
        if (tail_ == head_) {
            return 0;
        }
        if (tail_ > head_) {
            return tail_ - head_;
        } else {
            return (capacity - head_) + tail_;
        }
    }

    void push_back(T const &value) {
        ensure_capacity();
        buffer[tail_] = value;
        ++tail_;
        tail_ %= capacity;
    }

    void push_front(T const &value) {
        ensure_capacity();
        head_ = head_ == 0 ? (capacity - 1) : (head_ - 1);
        buffer[head_] = value;
    }

    void pop_back() {
        tail_ = tail_ == 0 ? (capacity - 1) : (tail_ - 1);
    }

    void pop_front() {
        head_++;
        head_ %= capacity;
    }

    T &front() {
        return operator[](0);
    }

    T &back() {
        return operator[](size() - 1);
    }

    T const &front() const {
        return operator[](0);
    }

    T const &back() const {
        return operator[](size() - 1);
    }

    T &operator[](size_t ind) {
        if (head_ + ind < capacity) {
            return buffer[head_ + ind];
        } else {
            return buffer[ind - (capacity - head_)];
        }
    }

    T const &operator[](size_t ind) const {
        if (head_ + ind < capacity) {
            return buffer[head_ + ind];
        } else {
            return buffer[ind - (capacity - head_)];
        }
    }

    iterator insert(const_iterator pos, T const &value) {
//        ensure_capacity();
        if (dist(head_, pos - begin()) + 1 < dist(pos - begin(), tail_)) {
            push_front(value);
            for (size_t i = 0; i < pos - begin(); i++) {
                std::swap(operator[](i), operator[](i + 1));
            }
        } else {
            push_back(value);
            for (size_t i = size() - 1; i > pos - begin(); i--) {
                std::swap(operator[](i), operator[](i - 1));
            }
        }
        return iterator(buffer, head_, tail_, capacity, pos - begin());
    }

    iterator erase(const_iterator pos) {
        if (dist(head_, pos - begin()) + 1 < dist(pos - begin(), tail_)) {
            for (size_t i = pos - begin(); i > 0; i--) {
                std::swap(operator[](i), operator[](i - 1));
            }
            pop_front();
        } else {
            for (size_t i = pos - begin(); i < size() - 1; i++) {
                std::swap(operator[](i), operator[](i + 1));
            }
            pop_back();
        }
        return iterator(buffer, head_, tail_, capacity, pos - begin());
    }

    iterator begin() {
        return iterator(buffer, head_, tail_, capacity, 0);
    }

    const_iterator begin() const {
        return const_iterator(buffer, head_, tail_, capacity, 0);
    }

    iterator end() {
        return iterator(buffer, head_, tail_, capacity, size());
    }

    const_iterator end() const {
        return const_iterator(buffer, head_, tail_, capacity, size());
    }


    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

private:
    size_t dist(size_t a, size_t b) {
        if (b == a) {
            return 0;
        }
        if (b > a) {
            return b - a;
        } else {
            return (capacity - a) + b;
        }
    }

    void ensure_capacity() {
        if (capacity - size() > 1) {
            return;
        }
        size_t new_capacity = capacity * 2;
        T *new_buff = static_cast<T *>(operator new[](sizeof(T) * new_capacity));

        if (tail_ > head_) {
            for (size_t i = head_; i < tail_; i++) {
                new_buff[i - head_] = buffer[i];
            }
        } else {
            size_t j = 0;
            for (size_t i = head_; i < capacity; i++) {
                new_buff[j++] = buffer[i];
            }
            for (size_t i = 0; i < tail_; i++) {
                new_buff[j++] = buffer[i];
            }
        }

        tail_ = size();
        head_ = 0;
        capacity = new_capacity;
        delete[] buffer;
        buffer = new_buff;
    }

    size_t head_ = 0;
    size_t tail_ = 0;
    size_t capacity = 0;

    T *buffer;
};

template<typename U>
struct Iterator {
public:
    using difference_type = std::ptrdiff_t;
    using value_type = U;
    using pointer = U *;
    using reference = U &;
    using iterator_category = std::random_access_iterator_tag;

    friend circ_buff<U>;
//    friend circ_buff<std::remove_const<U>>;
    friend Iterator<const U>;


    Iterator() : buffer(nullptr), ind(0), head_(0), tail_(0), capacity(0) {}

    template <typename V>
    Iterator(Iterator<V> const& other,
             typename std::enable_if<std::is_same<V const, U>::value && std::is_const<U>::value>::type* = nullptr)
    :buffer(other.buffer), ind(other.ind) {

    }

    template<typename W>
    Iterator(Iterator<W> &other) : buffer(other.buffer), ind(other.ind), head_(other.head_), tail_(other.tail_) {}

    Iterator &operator++() {
        ind++;
        return *this;
    }

    Iterator operator++(int) {
        Iterator tmp(buffer, ind);
        ++(*this);
        return tmp;
    }

    Iterator &operator--() {
        ind--;
        return *this;
    }

    Iterator operator--(int) {
        Iterator tmp(buffer, ind);
        --(*this);
        return tmp;
    }

    U &operator*() const {
        if (head_ + ind < capacity) {
            return buffer[head_ + ind];
        } else {
            return buffer[ind - (capacity - head_)];
        }
    }

    U *operator->() {
        return &buffer[ind];
    }

    bool operator==(Iterator const &x) {
        return buffer == x.buffer && ind == x.ind;
    }

    bool operator!=(Iterator const &x) {
        return buffer != x.buffer || ind != x.ind;
    }

    bool operator<(Iterator const &b) {
        return ind < b.ind;
    }

    bool operator<=(Iterator const &b) {
        return ind <= b.ind;
    }

    bool operator>(Iterator const &b) {
        return ind > b.ind;
    }

    bool operator>=(Iterator const &b) {
        return ind >= b.ind;
    }

    Iterator& operator+=(ptrdiff_t n) {
        ind += n;
        return *this;
    }

    Iterator& operator-=(ptrdiff_t n) {
        ind -= n;
        return *this;
    }

    ptrdiff_t operator-(Iterator a) {
        return ind - a.ind;
    }

    template <typename X>
    friend Iterator<X> operator+(difference_type n, Iterator<X> a);

    template <typename X>
    friend Iterator<X> operator+(Iterator<X> a, ptrdiff_t n);

    template <typename X>
    friend Iterator<X> operator-(difference_type n, Iterator<X> a);

    template <typename X>
    friend Iterator<X> operator-(Iterator<X> a, ptrdiff_t n);

private:
    Iterator(U *buffer, size_t head, size_t tail, size_t capacity, size_t ind) : buffer(buffer), capacity(capacity), head_(head), tail_(tail), ind(ind) {}

    U *buffer;
    size_t head_;
    size_t tail_;
    size_t capacity;
    size_t ind;
};

template <typename U>
Iterator<U> operator+(ptrdiff_t n, Iterator<U> a) {
    a.ind += n;
    return a;
}

template <typename U>
Iterator<U> operator+(Iterator<U> a, ptrdiff_t n) {
    a.ind += n;
    return a;
}

template <typename U>
Iterator<U> operator-(ptrdiff_t n, Iterator<U> a) {
    a.ind -= n;
    return a;
}

template <typename U>
Iterator<U> operator-(Iterator<U> a, ptrdiff_t n) {
    a.ind -= n;
    return a;
}
#endif //CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
