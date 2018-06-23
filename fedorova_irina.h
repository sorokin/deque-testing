//
// Created by irina on 22.06.18.
//

#ifndef CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_CIRCULAR_BUFFER_H

#include <assert.h>
#include <cstddef>

namespace my {
    template<typename T>
    struct circular_buffer;

    template<typename R>
    void swap(circular_buffer<R> &a, circular_buffer<R> &b) noexcept {
        std::swap(a.capacity, b.capacity);
        std::swap(a.size_, b.size_);
        std::swap(a.data, b.data);
        std::swap(a.start, b.start);
    }

    template<typename T>
    struct circular_buffer {

        template<typename V>
        struct iterator_buf : std::iterator<std::random_access_iterator_tag, V> {
            friend circular_buffer;

        private:
            T *ptr;
            T *extreme_left, *extreme_right;

            iterator_buf(T *ptr, T *extreme_left, T *extreme_right) :
                    ptr(ptr),
                    extreme_left(extreme_left),
                    extreme_right(extreme_right) {}

        public:
            explicit iterator_buf() {}

            template<typename W>
            iterator_buf(iterator_buf<W> const &other) : ptr(other.ptr),
                                                         extreme_left(other.extreme_left),
                                                         extreme_right(other.extreme_right) {}


            V &operator*() {
                return *ptr;
            }

            V const &operator*() const {
                return *ptr;
            }

            V const &operator->() const {
                return &operator*();
            }

            template<typename W>
            iterator_buf &operator=(iterator_buf<W> const &other) {
                iterator_buf tmp(other);
                std::swap(tmp.ptr, ptr);
                std::swap(tmp.extreme_left, extreme_left);
                std::swap(tmp.extreme_right, extreme_right);
                return *this;
            }

            iterator_buf &operator++() {
                ++ptr;

                if (ptr == extreme_right) {
                    ptr = extreme_left;
                }
                return *this;
            }

            iterator_buf &operator--() {
                if (ptr == extreme_left) {
                    ptr = extreme_right;
                }

                --ptr;
                return *this;
            }

            iterator_buf operator++(int) {
                iterator_buf ret = *this;
                ++(*this);
                return ret;
            }

            iterator_buf operator--(int) {
                iterator_buf ret = *this;
                --(*this);
                return ret;
            }

            iterator_buf &operator+=(ptrdiff_t shift) {
                ptrdiff_t pos = ptr - extreme_left;
                pos = (pos + shift) % (extreme_right - extreme_left);
                ptr = extreme_left + pos;
                return *this;
            }

            iterator_buf &operator-=(ptrdiff_t shift) {
                ptrdiff_t pos = ptr - extreme_left;
                pos = (pos - shift + (extreme_right - extreme_left)) % (extreme_right - extreme_left);
                ptr = extreme_left + pos;
                return *this;
            }

            iterator_buf operator+(ptrdiff_t shift) {
                iterator_buf ret(ptr, extreme_left, extreme_right);
                ret += shift;
                return ret;
            }

            iterator_buf operator-(ptrdiff_t shift) {
                iterator_buf ret(ptr, extreme_left, extreme_right);
                ret -= shift;
                return ret;
            }

            friend bool operator==(iterator_buf const &it1, iterator_buf const &it2) {
                return it1.ptr == it2.ptr;
            }

            friend bool operator!=(iterator_buf const &it1, iterator_buf const &it2) {
                return it1.ptr != it2.ptr;
            }

            friend ptrdiff_t operator-(iterator_buf const &it1, iterator_buf const &it2) {
                return it1.ptr - it2.ptr;
            }
        };

    private:
        size_t capacity;
        size_t size_;
        size_t start;
        T *data;

        void resize_x2() {
            circular_buffer tmp(*this, capacity * 2);
            swap(*this, tmp);
        }

        circular_buffer(circular_buffer const &other, size_t new_capacity) {
            data = (T*) operator new(new_capacity * sizeof(T));
            assert(new_capacity > other.capacity);
            capacity = new_capacity;
            size_ = other.size_;
            start = 0;
            size_t i = 0;
            try {
                for (auto it = other.begin(); it != other.end(); ++it) {
                    new(data + i) T(*it);
                    ++i;
                }
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    (data + j) -> ~T();
                }
                operator delete(data);
            }
        }

    public:

        typedef iterator_buf<T> iterator;
        typedef iterator_buf<const T> const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        iterator begin() {
            return iterator(data + start, data, data + capacity);
        }

        iterator end() {
            return iterator(data + (start + size_) % capacity, data, data + capacity);
        }

        const_iterator begin() const {
            return const_iterator(data + start, data, data + capacity);
        }

        const_iterator end() const {
            return const_iterator(data + (start + size_) % capacity, data, data + capacity);
        }

        circular_buffer() : capacity(2), size_(0), start(0), data((T*) operator new(capacity * sizeof(T))) {}

        circular_buffer(circular_buffer const& other) : capacity(other.capacity), size_(other.size_), start(0) {
            data = (T*) operator new(other.capacity * sizeof(T));
            size_t i = 0;
            try {
                for (auto it = other.begin(); it != other.end(); ++it) {
                    new(data + i) T(*it);
                    ++i;
                }
            } catch (...) {
                for (size_t j = 0; j < i; ++j) {
                    (data + j) -> ~T();
                }
                operator delete(data);
            }
        }

        ~circular_buffer() {
            for (size_t i = 0; i < size_; ++i) {
                (data + (start + i) % capacity) -> ~T();
            }
            void* p = (void*) data;
            operator delete(p);
        }

        circular_buffer& operator=(circular_buffer const& other) {
            circular_buffer tmp(other);
            swap(*this, tmp);
            return *this;
        }

        bool empty() const noexcept {
            return size_ == 0;
        }

        void clear() noexcept {
            for (auto it = begin(); it != end(); ++it) {
                pop_front();
            }
        }

        T& back() noexcept {
            assert(size_ && "can't back, size == 0");
            return data[(start + size_ - 1) % capacity];
        }

        T const& back() const noexcept {
            assert(size_ && "can't back, size == 0");
            return data[(start + size_ - 1) % capacity];
        }

        void pop_back() {
            assert(size_ && "can't push_back(), size == 0");
            --size_;
            (data + (start + size_) % capacity) -> ~T();
        }

        void push_back(T const& val) {
            if (size_ + 1 >= capacity) {
                resize_x2();
            }
            new(data + (start + size_) % capacity) T(val);
            ++size_;
        }

        T& front() noexcept {
            assert(size_ && "can't front, size == 0");
            return data[start];
        }

        T const& front() const noexcept {
            assert(size_ && "can't front, size == 0");
            return data[start];
        }

        void pop_front() noexcept {
            assert(size_ && "can't pop_front(), size == 0");
            (data + start) -> ~T();
            start = (start + 1) % capacity;
            --size_;
        }

        void push_front(T const& val) {
            if (size_ + 1 >= capacity) {
                resize_x2();
            }
            start = (start + capacity - 1) % capacity;
            new(data + start) T(val);
            ++size_;
        }

        size_t size() const {
            return size_;
        }

        T& operator[](size_t ind) {
            assert(ind < size_ && "can'r [], ind > size");
            return data[(start + ind) % capacity];
        }

        T const& operator[](size_t ind) const {
            assert(ind < size_ && "can'r [], ind > size");
            return data[(start + ind) % capacity];
        }

        iterator erase(iterator it) {
            iterator ret;
            if (it - begin() < (int) size_ / 2) {
                ret = it + 1;
                while (it != begin()) {
                    iterator next = it--;
                    std::swap(*it, *next);
                    it = next;
                }
                pop_front();
            } else {
                ret = it;
                it++;
                while (it != end()) {
                    auto prev = it - 1;
                    std::swap(*it, *prev);
                    it++;
                }
                pop_back();
            }
            return ret;
        }

        iterator insert(iterator it, T const& val) {
            iterator ret;
            size_t pos = it - begin();
            if (pos < size_ / 2) {
                push_front(val);
                for (size_t i = 0; i < pos; ++i) {
                    std::swap(data[(start + i) % capacity], data[(start + i + 1) % capacity]);
                }
                ret = iterator(data + (start + pos) % capacity, data, data + capacity);
            } else {
                push_back(val);
                for (size_t i = size_ - 1; i > pos; --i) {
                    std::swap(data[(start + i) % capacity], data[(start + i - 1) % capacity]);
                }
                ret = iterator(data + (start + pos) % capacity, data, data + capacity);
            }
            return ret;
        }

        friend void swap<T>(circular_buffer &, circular_buffer &) noexcept;
    };
}

#endif //CIRCULAR_BUFFER_CIRCULAR_BUFFER_H
