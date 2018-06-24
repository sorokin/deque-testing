//
// Created by Nikitos on 23.06.18.
//

#ifndef DEC_my_deq_H
#define DEC_my_deq_H

#include <iostream>
#include <assert.h>

const int START_CAPACITY = 8;

template <typename T>
struct my_deq {
private:
    template <typename U>
    struct my_iterator {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = U;
        using pointer = U*;
        using reference = U&;

        my_iterator(T* begin, size_t index, size_t left, size_t capacity):
                begin(begin),
                index(index),
                left(left),
                capacity(capacity)
        {}

        my_iterator(my_iterator const & other):
                begin(other.begin),
                index(other.index),
                left(other.left),
                capacity(other.capacity)
        {}

        template <typename V>
        my_iterator(my_iterator<V> const & other, typename std::enable_if<std::is_same<V const, U>::value && std::is_const<U>::value>::type* = nullptr):
                begin(other.begin),
                index(other.index),
                left(other.left),
                capacity(other.capacity)
        {}

        ~my_iterator() = default;

        my_iterator & operator=(my_iterator const & other) {
            my_iterator copy(other);
            this->swap(copy);
            return *this;
        }

        my_iterator& operator++() {
            ++index;
            return *this;
        }

        my_iterator& operator--() {
            --index;
            return *this;
        }

        my_iterator operator++(int) {
            my_iterator answer = *this;
            ++(*this);
            return answer;
        }

        my_iterator operator--(int) {
            my_iterator answer = *this;
            --(*this);
            return answer;
        }

        reference operator*() const {
            return *(begin + get_pos());
        }

        pointer operator->() const {
            return begin + get_pos();
        }

        friend bool operator==(my_iterator const& it, my_iterator const& other) {
            return it.begin == other.begin && it.left == other.left && it.capacity == other.capacity && it.index == other.index;
        }

        friend bool operator!=(my_iterator const& it, my_iterator const& other) {
            return !(it == other);
        }

        friend size_t operator-(my_iterator const& it,my_iterator const& other) {
            return it.index - other.index;
        }

        friend my_iterator operator+(my_iterator const& it, size_t delta) {
            return my_iterator(it.begin, it.index + delta, it.left, it.capacity);
        }

        friend my_iterator operator-(my_iterator const& it, size_t delta) {
            return it + (-delta);
        }

        friend my_iterator operator+(size_t delta, my_iterator const& it) {
            return it + delta;
        }

        friend my_iterator operator-(size_t delta, my_iterator const& it) {
            return it - delta;
        }

        friend my_iterator& operator+=(my_iterator& it, size_t delta) {
            it = it + delta;
            return it;
        }

        friend my_iterator& operator-=(my_iterator& it, size_t delta) {
            it = it - delta;
            return it;
        }

        void swap(my_iterator & other) {
            using std::swap;
            swap(begin, other.begin);
            swap(left, other.left);
            swap(capacity, other.capacity);
            swap(index, other.index);
        }


        bool operator>(my_iterator const & other) {
            return left > other.left;
        }

        bool operator<(my_iterator const & other) {
            return *this != other && !(*this > other);
        }

        bool operator>=(my_iterator const & other) {
            return !(*this < other);
        }

        bool operator<=(my_iterator const & other) {
            return !(*this > other);
        }

        //------------------------------------------------------------------
        //------------------ METHODS FOR MY IMPLEMENTATIONS ----------------
        //------------------------------------------------------------------

    private:
        T* begin;
        size_t index, left;
        size_t capacity;

        size_t get_pos() const {
            return (left + index) % capacity;
        }

        friend struct my_deq;
    };

public:
    using iterator = my_iterator<T>;
    using const_iterator = my_iterator<T const>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    my_deq();
    my_deq(size_t capacity);
    my_deq(my_deq const &);
    my_deq& operator=(my_deq const &);
    ~my_deq();

    void push_back(T const &);
    void push_front(T const &);

    void pop_back();
    void pop_front();

    T & front() const;
    T & back() const;

    T & operator[](size_t) const;

    iterator insert(const_iterator, T const &);
    iterator erase(const_iterator);

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;

    reverse_iterator rend();
    const_reverse_iterator rend() const;


    size_t size() const;
    bool empty() const;
    void clear();

    void swap(my_deq &);

private:

    size_t head_;
    size_t tail_;
    size_t size_;
    size_t capacity_;
    T * data_;

    //------------------------------------------------------------------
    //------------------ METHODS FOR MY IMPLEMENTATIONS ----------------
    //------------------------------------------------------------------

    void ensure_capacity(size_t);
    void fixup();

};

template<typename T>
my_deq<T>::my_deq():
        head_(0),
        tail_(0),
        size_(0),
        capacity_(0),
        data_(nullptr)
{}

template<typename T>
my_deq<T>::my_deq(size_t capacity):
        head_(0),
        tail_(0),
        size_(0),
        capacity_(capacity),
        data_(static_cast<T*>(operator new(capacity * sizeof(T))))
{}

template<typename T>
my_deq<T>::my_deq(my_deq const & other):
        head_(other.head_),
        tail_(other.tail_),
        size_(other.size_),
        capacity_(other.capacity_),
        data_(nullptr)
{
    if (capacity_ == 0) {
        return;
    }

    data_ = static_cast<T*>(operator new(capacity_ * sizeof(T)));

    for (size_t i = 0, j = head_; i < size_; ++i, j = (j + 1) % capacity_) {
        try {
            new (data_ + j) T(other.data_[j]);
        } catch(...) {
            for (size_t j = 0, k = head_; j < i; ++j, k = (k + 1) % capacity_) {
                data_[k].~T();
            }

            operator delete(data_);
            throw;
        }
    }
}

template<typename T>
my_deq<T>& my_deq<T>::operator=(my_deq const & other) {
    if (this != &other) {
        my_deq copy(other);
        this->swap(copy);
    }

    return *this;
}

template<typename T>
my_deq<T>::~my_deq() {
    if (capacity_ == 0)
        return;

    for (size_t i = 0, pos = head_; i < size_; ++i, pos = (pos + 1) % capacity_)
        data_[pos].~T();
    operator delete(data_);
}

template<typename T>
void my_deq<T>::push_back(const T & value) {
    ensure_capacity(size_ + 2);

    if (size_ == 0) {
        new (data_) T(value);
    } else {
        size_t copy_tail = (tail_ + 1) % capacity_;
        new (data_ + copy_tail) T(value);
        tail_ = copy_tail;
    }

    size_++;
}

template<typename T>
void my_deq<T>::push_front(const T & value) {
    ensure_capacity(size_ + 2);

    if (size_ == 0) {
        new (data_) T(value);
    } else {
        size_t copy_head = (head_ != 0 ? head_ - 1 : capacity_ - 1);
        new (data_ + copy_head) T(value);
        head_ = copy_head;
    }

    size_++;
}

template<typename T>
void my_deq<T>::pop_back() {
    assert(size_ > 0);

    data_[tail_].~T();
    tail_ = (tail_ != 0? tail_ - 1 : capacity_ - 1);
    size_--;
    fixup();
}

template<typename T>
void my_deq<T>::pop_front() {
    assert(size_ > 0);

    data_[head_].~T();
    head_ = (head_ + 1) % capacity_;
    size_--;
    fixup();
}

template<typename T>
T &my_deq<T>::front() const {
    assert(size_ > 0);

    return data_[head_];
}

template<typename T>
T &my_deq<T>::back() const {
    assert(size_ > 0);

    return data_[tail_];
}

template<typename T>
void my_deq<T>::ensure_capacity(size_t size) {
    if (size <= capacity_) {
        return;
    }

    if (capacity_ == 0) {
        capacity_ = 2;
        data_ = static_cast<T*>(operator new(capacity_ * sizeof(T)));
        return;
    }

    my_deq<T> material(2 * capacity_ - 1);

    for (size_t i = 0, pos = head_; i < size_; ++i, pos = (pos + 1) % capacity_) {
        material.push_back(data_[pos]);
    }

    this->swap(material);
}

template<typename T>
void my_deq<T>::fixup() {
    if (empty()) {
        head_ = tail_ = 0;
    }
}

template<typename T>
T &my_deq<T>::operator[](size_t pos) const {
    assert(pos < size_);

    return data_[(head_ + pos) % capacity_];
}

template<typename T>
typename my_deq<T>::iterator my_deq<T>::insert(typename my_deq<T>::const_iterator it, const T & value) {
    if (empty()) {
        push_back(value);
        return begin();
    }
    size_t pos = it.index;
    if (pos <= size_ - pos) {
        push_front(value);
        iterator cur = begin(), target = cur + pos;;
        ++cur;
        while ((cur - 1) != target) {
            *(cur - 1) = *cur;
            cur++;
        }
        *target = value;
    } else {
        iterator cur = begin() + pos;
        T homeless_val = value;
        while (cur != end()) {
            std::swap(*cur, homeless_val);
            ++cur;
        }
        push_back(homeless_val);
    }
    return begin() + pos;
}

template<typename T>
typename my_deq<T>::iterator my_deq<T>::erase(typename my_deq<T>::const_iterator it) {
    assert(size_ > 0);

    size_t pos = it.index;

    if (pos <= size_ - pos) {
        iterator cur = begin() + pos;;
        while (cur != begin()) {
            *cur = *(cur - 1);
            --cur;
        }
        pop_front();
    } else {
        iterator cur = begin() + pos;
        while (cur != (end() - 1)) {
            *cur = *(cur + 1);
            cur++;
        }
        pop_back();
    }

    return begin() + pos;
}

template<typename T>
size_t my_deq<T>::size() const {
    return size_;
}

template<typename T>
bool my_deq<T>::empty() const {
    return size_ == 0;
}

template<typename T>
void my_deq<T>::clear() {
    my_deq cl;
    this->swap(cl);
}

template<typename T>
void my_deq<T>::swap(my_deq & other) {
    using std::swap;
    swap(data_, other.data_);
    swap(size_, other.size_);
    swap(capacity_, other.capacity_);
    swap(head_, other.head_);
    swap(tail_, other.tail_);
}

template<typename T>
typename my_deq<T>::iterator my_deq<T>::begin() {
    return iterator(data_, 0, head_, capacity_);
}

template<typename T>
typename my_deq<T>::const_iterator my_deq<T>::begin() const {
    return const_iterator(data_, 0, head_, capacity_);
}

template<typename T>
typename my_deq<T>::iterator my_deq<T>::end() {
    return iterator(data_, size_, head_, capacity_);
}

template<typename T>
typename my_deq<T>::const_iterator my_deq<T>::end() const {
    return const_iterator(data_, size_, head_, capacity_);
}

template<typename T>
typename my_deq<T>::reverse_iterator my_deq<T>::rbegin() {
    return reverse_iterator(end());
}

template<typename T>
typename my_deq<T>::const_reverse_iterator my_deq<T>::rbegin() const {
    return const_reverse_iterator(end());
}

template<typename T>
typename my_deq<T>::reverse_iterator my_deq<T>::rend() {
    return reverse_iterator(begin());
}

template<typename T>
typename my_deq<T>::const_reverse_iterator my_deq<T>::rend() const {
    return const_reverse_iterator(begin());
}

template<typename T>
void swap(my_deq<T> & l, my_deq<T> & r) {
    l.swap(r);
}

#endif //DEC_my_deq_H
