#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

template <typename T>
class circular_buffer {
private:
    template <typename V>
    struct buffer_iterator;
    size_t _capacity, _size;
    size_t _head, _tail;
    T* arr;

    size_t getNextPos(size_t ind) const {
        if (_capacity == 0) {
            return 0;
        }
        return (ind + 1) % _capacity;
    }

    size_t getPrevPos(size_t ind) const {
        if (_capacity == 0) {
            return 0;
        }
        return (_capacity + ind - 1) % _capacity;
    }

    void memcp(T* src, T* dest) {
        for (size_t i = 0, j = _head; i < _size; ++i, ++j) {
            new (&dest[i]) T(src[getNextPos(j)]);
        }
    }

    void ensureCapacity(size_t newsize) {
        if (newsize <= _capacity) {
            return;
        }
        size_t newcapacity = (_capacity + 3) * 2;
        T* newarr = static_cast<T*>(operator new(sizeof(T) * newcapacity));
        memcp(arr, newarr);
        clear();
        arr = newarr;
        _capacity = newcapacity;
        _head = _capacity - 1;
        _tail = _size;
    }

public:
    using iterator = buffer_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_iterator = buffer_iterator<T const>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    circular_buffer() {
        _tail = _head = _capacity = _size = 0;
    }

    circular_buffer(circular_buffer const& rhs) {
        _capacity = rhs._capacity;
        _size = rhs._size;
        _head = rhs._head;
        _tail = rhs._tail;
        arr = static_cast<T*>(operator new(sizeof(T) * _capacity));
        memcp(rhs.arr, arr);
    }

    circular_buffer& operator=(circular_buffer const& rhs) {
        if (arr != rhs.arr) {
            clear();
            _capacity = rhs._capacity;
            _size = rhs._size;
            _head = rhs._head;
            _tail = rhs._tail;
            arr = static_cast<T*>(operator new(sizeof(T) * _capacity));
            memcp(rhs.arr, arr);
        }
        return *this;
    }

    ~circular_buffer() {
        clear();
    }

    bool empty() {
        return (_size == 0);
    }

    void clear() {
        if (_capacity != 0) {
            delete arr;
        }
    }

    size_t size() {
        return _size;
    }

    T operator[](size_t const index) {
        return arr[(_head + index + 1) % _capacity];
    }

    void push_front(T const& val) {
        ensureCapacity(_size + 1);
        new (&arr[_head]) T(val);
        _head = getPrevPos(_head);
        ++_size;
    }

    void pop_front() {
        _head = getNextPos(_head);
        --_size;
    }

    T& front() {
        return arr[getNextPos(_head)];
    }

    T const& front() const {
        return arr[getNextPos(_head)];
    }

    void push_back(T const& val) {
        ensureCapacity(_size + 1);
        new (&arr[_tail]) T(val);
        _tail = getNextPos(_tail);
        ++_size;
    }

    void pop_back() {
        _tail = getPrevPos(_tail);
        --_size;
    }

    T& back() {
        return arr[getPrevPos(_tail)];
    }

    T const& back() const {
        return arr[getPrevPos(_tail)];
    }

    void swap(circular_buffer& other) {
        std::swap(_capacity, other._capacity);
        std::swap(_size, other._size);
        std::swap(_head, other._head);
        std::swap(_tail, other._tail);
        std::swap(arr, other.arr);
    }

    iterator begin() {
        return iterator(0, arr, _head, _capacity);
    }

    const_iterator begin() const {
        return const_iterator(0, arr, _head, _capacity);
    }

    iterator end() {
        return iterator(_size, arr, _head, _capacity);
    }

    const_iterator end() const {
        return const_iterator(_size, arr, _head, _capacity);
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

    iterator insert(const_iterator, const T&);
    iterator erase(const_iterator pos);
};

template <typename T>
template <typename V>
class circular_buffer<T>::buffer_iterator
{
private:
    buffer_iterator(size_t ind, T* _a, size_t _s, size_t _e) {
        _head = _s;
        _capacity = _e;
        index = ind;
        arr = _a;
    }
    size_t index, _head, _capacity;
    T* arr;
public:
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = V;
    using pointer = V*;
    using reference = V&;

    buffer_iterator() {}
    buffer_iterator(buffer_iterator const& rhs) = default;

    template <typename U>
    buffer_iterator(buffer_iterator<U> const& rhs, typename std::enable_if<std::is_same<U const, V>::value && std::is_const<V>::value>::type* = nullptr) {
        index = rhs.index;
        arr = rhs.arr;
        _head = rhs._head;
        _capacity = rhs._capacity;
    }

    buffer_iterator& operator=(buffer_iterator const& rhs) = default;

    reference operator*() const {
        return arr[(_head + index + 1) % _capacity];
    }

    pointer operator->() const {
        return &arr[(_head + index + 1) % _capacity];
    }

    buffer_iterator& operator++() {
        ++index;
        return *this;
    }

    buffer_iterator operator++(int) {
        buffer_iterator copy = *this;
        ++*this;
        return copy;
    }

    buffer_iterator& operator--() {
        --index;
        return *this;
    }

    buffer_iterator operator--(int) {
        buffer_iterator copy = *this;
        --*this;
        return copy;
    }

    buffer_iterator operator-=(size_t i) {
        index -= i;
        return *this;
    }

    friend buffer_iterator operator-(buffer_iterator const& a, size_t i) {
        buffer_iterator copy = a;
        copy -= i;
        return copy;
    }

    friend buffer_iterator operator+(size_t i, buffer_iterator const& a) {
        buffer_iterator copy = a;
        copy += i;
        return copy;
    }

    buffer_iterator operator+=(size_t i) {
        index += i;
        return *this;
    }

    friend buffer_iterator operator+(buffer_iterator const& a, size_t i) {
        buffer_iterator copy = a;
        copy += i;
        return copy;
    }

    friend difference_type operator-(buffer_iterator const& a, buffer_iterator const& b) {
        return (static_cast<difference_type>(a.index) - static_cast<difference_type>(b.index));
    }

    friend bool operator>=(buffer_iterator const& a, buffer_iterator const& b) {
        return (a.index >= b.index);
    }

    friend bool operator<=(buffer_iterator const& a, buffer_iterator const& b) {
        return (a.index <= b.index);
    }

    friend bool operator<(buffer_iterator const& a, buffer_iterator const& b) {
        return (a.index < b.index);
    }

    friend bool operator>(buffer_iterator const& a, buffer_iterator const& b) {
        return (a.index > b.index);
    }

    friend bool operator!=(buffer_iterator const& a, buffer_iterator const& b) {
        return a.index != b.index;
    }

    friend bool operator==(buffer_iterator const& a, buffer_iterator const& b) {
        return a.index == b.index;
    }

    friend class circular_buffer;
};

template <typename T>
circular_buffer<T>::buffer_iterator<T> circular_buffer<T>::insert(buffer_iterator<T const> pos, T const& val) {
    ensureCapacity(_size + 1);
    buffer_iterator<T> temp(pos.index, arr, _head, _capacity);
    if (std::abs(pos - begin()) < std::abs(pos - end())) {
        push_front(val);
        auto cur = begin();
        while (cur != temp) {
            std::swap(*cur, *(cur + 1));
            ++cur;
        }
        *cur = val;
    } else {
        push_back(val);
        auto cur = end();
        while (cur != temp) {
            std::swap(*cur, *(cur - 1));
            --cur;
        }
        *cur = val;
    }
    buffer_iterator<T> ans(pos.index, arr, _head, _capacity);
    return ans;
}

template <typename T>
circular_buffer<T>::buffer_iterator<T> circular_buffer<T>::erase(buffer_iterator<T const> pos) {
    buffer_iterator<T> temp(pos.index, arr, _head, _capacity);
    if (std::abs(pos - begin()) < std::abs(pos - end())) {
        auto cur = temp;
        while (cur != begin()) {
            std::swap(*cur, *(cur - 1));
            --cur;
        }
        pop_front();
    } else {
        auto cur = temp;
        while (cur != end()) {
            std::swap(*cur, *(cur + 1));
            ++cur;
        }
        pop_back();
    }
    buffer_iterator<T> ans(pos.index, arr, _head, _capacity);
    return ans;
}

template<typename T>
void swap(circular_buffer<T>& a, circular_buffer<T>& b) {
    a.swap(b);
}

#endif
