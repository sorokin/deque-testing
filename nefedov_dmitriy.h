//
// Created by nefed on 24.06.2018.
//

#ifndef DEQUE_DEQUE_H
#define DEQUE_DEQUE_H

#include <cstring>
#include <cassert>

template<class T>
struct deque {
    template<class U = T>
    struct iterator : public std::iterator<std::random_access_iterator_tag, U> {
        iterator() = delete;

        iterator(const iterator<U> &other) = default;

        template<class N, typename = typename std::enable_if<
                std::is_const<U>::value && std::is_same<U, const N>::value, void>::type>
        iterator(const iterator<N> &other) : data(const_cast<U **>(other.data)),
                                             ind(other.ind),
                                             first(other.first),
                                             capacity(other.capacity),
                                             length(other.length) {}

        iterator &operator=(const iterator<U> &other) = default;

        template<class N, typename = typename std::enable_if<
                std::is_const<U>::value && std::is_same<U, const N>::value, void>::type>
        iterator &operator=(const iterator<N> &other) {
            data = const_cast<U **>(other.data);
            ind = other.ind;
            first = other.first;
            capacity = other.capacity;
            length = other.length;
            return *this;
        }

        U &operator*() {
            return *(data[(first + ind) % capacity]);
        }

        U &operator->() {
            return *(*this);
        }

        iterator &operator-=(int d) {
            assert(d <= ind);
            ind -= d;
            return *this;
        }

        iterator &operator+=(int d) {
            assert(d <= length - ind);
            ind += d;
            return *this;
        }

        iterator &operator--() {
            return (*this) -= 1;
        }

        iterator &operator++() {
            return (*this) += 1;
        }

        const iterator operator--(int) {
            iterator<U> r = *this;
            --*this;
            return r;
        }

        const iterator operator++(int) {
            iterator<U> r = *this;
            ++*this;
            return r;
        }

        iterator operator-(int d) {
            iterator<U> r = *this;
            return r -= d;
        }

        iterator operator+(int d) {
            iterator<U> r = *this;
            return r += d;
        }

        template<class N>
        bool operator==(iterator<N> b);

        template<class N>
        bool operator!=(iterator<N> b);

    private:
        U **data;
        size_t ind, first, capacity, length;

        iterator(U **data, size_t ind, size_t first, size_t capacity, size_t length) :
                data(data),
                ind(ind),
                first(first),
                capacity(capacity),
                length(length) {}

        friend struct deque;
    };

    deque();

    deque(const deque &other);

    deque &operator=(const deque &other);

    void push_back(T const &value);

    void push_front(T const &value);

    void pop_back() noexcept;

    void pop_front() noexcept;

    iterator<T> front() noexcept;

    iterator<T> back() noexcept;

    iterator<const T> cfront() const noexcept;

    iterator<const T> cback() const noexcept;

    std::reverse_iterator<iterator<T>> rfront() noexcept {
        return std::reverse_iterator<iterator<T>>(back());
    }

    std::reverse_iterator<iterator<T>> rback() noexcept {
        return std::reverse_iterator<iterator<T>>(front());
    }

    std::reverse_iterator<iterator<const T>> crfront() const noexcept {
        std::reverse_iterator<iterator<const T>>(cfront());
    }

    std::reverse_iterator<iterator<const T>> crback() const noexcept {
        return std::reverse_iterator<iterator<const T>>(cfront());
    }

    T &operator[](size_t i) noexcept;

    const T &operator[](size_t i) const noexcept;

    bool empty() const noexcept;

    void clear() noexcept;

    size_t size() const noexcept;

    iterator <T> insert(deque<T>::iterator<const T> pos, T const &value);

    iterator<T> erase(deque<T>::iterator<> pos);

    ~deque();

private:
    static const size_t START_SIZE = 128;

    T **data;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t length;

    size_t safe_dec(size_t i) const noexcept;

    size_t safe_inc(size_t i) const noexcept;

    void data_update() noexcept;

    size_t safe_ind(size_t i) const noexcept;

    template<class Y>
    friend void swap(deque<Y> a, deque<Y> b);
};


template<class T>
template<class U>
template<class N>
bool deque<T>::iterator<U>::operator==(deque::iterator<N> b) {
    return data == b.data && first == b.first && ind == b.ind;
}

template<class T>
template<class U>
template<class N>
bool deque<T>::iterator<U>::operator!=(deque::iterator<N> b) {
    return !((*this) == b);
}


template<class T>
deque<T>::deque() :
        data(new T *[START_SIZE]),
        capacity(START_SIZE),
        head(0),
        tail(0),
        length(0) {}

template<class T>
size_t deque<T>::safe_dec(size_t i) const noexcept {
    return i == 0 ? capacity - 1 : --i;
}

template<class T>
size_t deque<T>::safe_inc(size_t i) const noexcept {
    return ++i == capacity ? 0 : i;
}

template<class T>
void deque<T>::data_update() noexcept {
    auto old = data;
    data = new T *[capacity * 2];
    if (tail > head) {
        std::memcpy(data + head, data, sizeof(T *) * length);
    } else {
        size_t f_block = capacity - head;
        size_t s_block = length - f_block;

        std::memcpy(data + head, data, sizeof(T *) * f_block);
        std::memcpy(data + head, data + f_block, sizeof(T *) * s_block);
    }
    head = 0;
    tail = length;
    capacity *= 2;
    delete[] old;
}

template<class T>
void deque<T>::push_back(const T &value) {
    if (length + 1 == capacity) {
        data_update();
    }
    // if T(value) throw any exceptions invariant will not be broken and data wil be safe but not updated
    auto tmp = new T(value);
    data[tail] = tmp;
    tail = safe_inc(tail);
    ++length;
}

template<class T>
void deque<T>::push_front(const T &value) {
    if (length + 1 == capacity) {
        data_update();
    }
    // if T(value) throw any exceptions invariant will not be broken and data wil be safe but not updated
    auto tmp = new T(value);
    head = safe_dec(head);
    data[head] = tmp;
    ++length;
}

template<class T>
void deque<T>::pop_back() noexcept {
    tail = safe_dec(tail);
    delete data[tail];
    data[tail] = nullptr;
    --length;
}

template<class T>
void deque<T>::pop_front() noexcept {
    delete data[head];
    head = safe_inc(head);
    --length;
}

template<class T>
size_t deque<T>::safe_ind(size_t i) const noexcept {
    return head + i < capacity ? head + i : head + i - capacity;
}

template<class T>
T &deque<T>::operator[](size_t i) noexcept {
    assert(i < length);
    return *data[safe_ind(i)];
}

template<class T>
const T &deque<T>::operator[](size_t i) const noexcept {
    assert(i < length);
    return *data[safe_ind(i)];
}

template<class T>
bool deque<T>::empty() const noexcept {
    return length == 0;
}

template<class T>
void deque<T>::clear() noexcept {
    for (size_t i = head; i != tail; i = safe_inc(i)) {
        delete data[i];
    }
    head = 0;
    tail = 0;
    length = 0;
}

template<class T>
size_t deque<T>::size() const noexcept {
    return length;
}

template<class T>
deque<T>::~deque() {
    clear();
    delete[] data;
}

template<class T>
deque<T>::iterator<T> deque<T>::front() noexcept {
    return iterator<T>(data, 0, head, capacity, length);
}

template<class T>
deque<T>::iterator<T> deque<T>::back() noexcept {
    return iterator<T>(data, length, head, capacity, length);
}

template<class T>
deque<T>::iterator<const T> deque<T>::cfront() const noexcept {
    return iterator<const T>(const_cast<const T **>(data), 0, head, capacity, length);
}

template<class T>
deque<T>::iterator<const T> deque<T>::cback() const noexcept {
    return iterator<const T>(const_cast<const T **>(data), length, head, capacity, length);
}

template<class T>
deque<T>::deque(const deque &other) :
        data(new T *[other.capacity]),
        capacity(other.capacity),
        head(other.head),
        tail(other.tail),
        length(other.length) {
    for (size_t i = head; i != tail; i = safe_inc(i)) {
        data[i] = new T(other.data[i]);
    }
}

template<class T>
deque<T> &deque<T>::operator=(const deque &other) {
    clear();
    data = new T *[other.capacity];
    capacity = other.capacity;
    head = other.head;
    tail = other.tail;
    length = other.length;
    for (size_t i = head; i != tail; i = safe_inc(i)) {
        data[i] = new T(other.data[i]);
    }
}

template<class T>
deque<T>::iterator<T> deque<T>::insert(deque::iterator<const T> pos, const T &value) {
    size_t ind = pos.ind;
    if (2 * ind <= length) {
        push_front(value);
        for (size_t i = head; i != (head + ind) % capacity; i = safe_inc(i)) {
            std::swap(data[i], data[safe_inc(i)]);
        }
    } else {
        push_back(value);
        //--ind;
        for (size_t i = tail - 1; i != (head + ind - 1) % capacity; i = safe_dec(i)) {
            std::swap(data[i], data[safe_dec(i)]);
        }
    }
    return iterator<T>(data, ind, head, capacity, length);
}

template<class T>
void swap(deque<T> a, deque<T> b) {
    std::swap(a.head, b.head);
    std::swap(a.tail, b.tail);
    std::swap(a.length, b.length);
    std::swap(a.capacity, b.capacity);
    std::swap(a.data, a.data);
}

template<class T>
deque<T>::iterator<T> deque<T>::erase(deque::iterator<> pos) {
    size_t ind = pos.ind;
    if (2 * ind <= length) {
        for (size_t i = (head + ind) % capacity; i != head; i = safe_dec(i)) {
            std::swap(data[i], data[safe_dec(i)]);
        }
        pop_front();
    } else {
        for (size_t i = (head + ind) % capacity; i != safe_dec(tail); i = safe_inc(i)) {
            std::swap(data[i], data[safe_inc(i)]);
        }
        pop_back();
    }
    return iterator<T>(data, ind, head, capacity, length);
}


#endif //DEQUE_DEQUE_H
