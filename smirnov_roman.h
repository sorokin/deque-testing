//
// Created by roman on 22.06.18.
//

#ifndef EXAM_CIRCULAR_BUFFER_H
#define EXAM_CIRCULAR_BUFFER_H

#include <iterator>
#include <cstddef>

template<typename T>
struct circular_buffer
{
public:
    circular_buffer() noexcept
        : buffer_start(nullptr), buffer_size(0), begin_shift(0), end_shift(0), _size(0)
    {};

    explicit circular_buffer(ptrdiff_t buffer_size)
        : buffer_start(reinterpret_cast<T *>(new char[sizeof(T) * (buffer_size + 1)])),
          buffer_size(buffer_size + 1),
          begin_shift(0),
          end_shift(0),
          _size(0)
    {};

    circular_buffer(const circular_buffer &other)
        : buffer_size(other.buffer_size), begin_shift(0), end_shift(other._size), _size(other._size)
    {
        buffer_start = reinterpret_cast<T *>(new char[sizeof(T) * other.buffer_size]);
        size_t ind = 0;
        for (T &i : other) {
            new(buffer_start + ind++)T(i);
        }
    }

    circular_buffer &operator=(const circular_buffer &other)
    {
        circular_buffer tmp(other);
        swap(tmp);
        return *this;
    }

    ~circular_buffer()
    {
        clear();
        delete[] reinterpret_cast<char *>(buffer_start);
    }

    template<typename S>
    struct my_iterator
    {
        friend circular_buffer;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef std::ptrdiff_t difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

    public:
        my_iterator(T *start, ptrdiff_t size, ptrdiff_t shift, ptrdiff_t begin_pos)
            : buffer_start(start), buffer_size(size), shift(shift), begin_pos(begin_pos)
        {}

        my_iterator(const my_iterator &) = default;

        template<typename F>
        my_iterator(const my_iterator<F> &other,
                    typename std::enable_if<
                        std::is_same<const F, S>::value && std::is_const<S>::value>::type * = nullptr)
            :
            buffer_start(other.buffer_start),
            buffer_size(other.buffer_size),
            shift(other.shift),
            begin_pos(other.begin_pos)
        {}

        my_iterator &operator=(const my_iterator &) = default;

        T &operator*()
        {
            return *(get_right_ptr());
        }
        T *operator->()
        {
            return get_right_ptr();
        }

        my_iterator &operator++()
        {
            ++shift;
            return *this;
        }
        my_iterator &operator--()
        {
            --shift;
            return *this;
        }

        my_iterator operator++(int)
        {
            my_iterator copy = *this;
            ++*this;
            return copy;
        }
        my_iterator operator--(int)
        {
            my_iterator copy = *this;
            --*this;
            return copy;
        }

        my_iterator &operator+=(ptrdiff_t dif)
        {
            shift += dif;
            return *this;
        }
        my_iterator &operator-=(ptrdiff_t dif)
        {
            return *this += -dif;
        }

        friend bool operator>=(const my_iterator &a, const my_iterator &b)
        {
            if ((a.get_right_ptr() >= a.get_begin_ptr() && b.get_right_ptr() >= b.get_begin_ptr())
                || (a.get_right_ptr() < a.get_begin_ptr() && b.get_right_ptr() < b.get_begin_ptr())) {
                return a.get_right_ptr() >= b.get_right_ptr();
            }
            else {
                return a.get_right_ptr() < b.get_right_ptr();
            }
        }
        friend bool operator<(const my_iterator &a, const my_iterator &b)
        {
            return !(a >= b);
        }
        friend bool operator<=(const my_iterator &a, const my_iterator &b)
        {
            if (a.get_right_ptr() >= a.get_begin_ptr() && b.get_right_ptr() >= b.get_begin_ptr()
                || a.get_right_ptr() < a.get_begin_ptr() && b.get_right_ptr() < b.get_begin_ptr()) {
                if (a.get_right_ptr() <= b.get_right_ptr()) {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                if (a.get_right_ptr() > b.get_right_ptr()) {
                    return true;
                }
                else {
                    return false;
                }
            }
        }
        friend bool operator>(const my_iterator &a, const my_iterator &b)
        {
            return !(a <= b);
        }

        my_iterator operator+(ptrdiff_t dif)
        {
            my_iterator tmp = *this;
            tmp += dif;
            return tmp;
        }
        my_iterator operator-(ptrdiff_t dif)
        {
            my_iterator tmp = *this;
            tmp -= dif;
            return tmp;
        }
        ptrdiff_t operator-(const my_iterator &other)
        {
            return (get_begin_ptr() + shift - other.get_begin_ptr() - other.shift) % (buffer_size ? buffer_size : 1);
        }

        friend bool operator==(const my_iterator &a, const my_iterator &b)
        {
            return (a.get_right_ptr()) == (b.get_right_ptr());
        }
        friend bool operator!=(const my_iterator &a, const my_iterator &b)
        {
            return !(a == b);
        }
    private:
        T *get_right_ptr() const
        {
            return buffer_start + (begin_pos + buffer_size + shift) % (buffer_size ? buffer_size : 1);
        }
        T *get_begin_ptr() const
        {
            return buffer_start + (begin_pos) % (buffer_size ? buffer_size : 1);
        }

    private:
        T *buffer_start;
        ptrdiff_t buffer_size;
        ptrdiff_t shift;
        ptrdiff_t begin_pos;
    };

    typedef my_iterator<T> iterator;
    typedef my_iterator<const T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
    iterator erase(const_iterator it)
    {
        ptrdiff_t pos = it - begin();
        iterator tmp = begin() + pos;

        for (ptrdiff_t i = pos + 1; i < _size; ++i) {
            (*this)[i - 1] = (*this)[i];
        }
        end_shift--;
        if (end_shift < 0) end_shift = buffer_size - 1;
        (buffer_start + end_shift)->~T();
        _size--;
        return tmp;
    }

    bool empty() const
    {
        return _size == 0;
    }

    void swap(circular_buffer &other)
    {
        std::swap(buffer_start, other.buffer_start);
        std::swap(buffer_size, other.buffer_size);
        std::swap(begin_shift, other.begin_shift);
        std::swap(end_shift, other.end_shift);
        std::swap(_size, other._size);
    }

    void clear()
    {
        for (T &i : *this) {
            (&i)->~T();
        }
    }

    iterator insert(const_iterator it, T const &item)
    {
        ptrdiff_t pos = it - begin();
        ensure_capacity(_size + 1);
        iterator tmp = iterator(buffer_start, buffer_size, pos, begin_shift);
        for (ptrdiff_t i = _size - pos - 1; i >= 0; --i) {
            new(buffer_start + (begin_shift + pos + i + 1) % buffer_size)T(*(buffer_start
                + (begin_shift + pos + i) % buffer_size));
            (buffer_start + (begin_shift + i) % buffer_size)->~T();
        }
        new(&*(tmp))T(item);
        *tmp = item;
        _size++;
        inc_end_shift();
        return tmp;
    }

    iterator push_back(T item)
    {
        return insert(end(), item);
    }

    iterator push_front(T item)
    {
        return insert(begin(), item);
    }

    T &front()
    {
        return (*this)[0];
    }
    const T &front() const
    {
        return (*this)[0];
    }
    const T &back() const
    {
        return (*this)[_size - 1];
    }

    void pop_back()
    {
        end_shift--;
        _size--;
        if (end_shift >= buffer_size) end_shift = buffer_size - 1;
        end()->~T();
    }
    void pop_front()
    {
        _size--;
        begin()->~T();
        begin_shift++;
        begin_shift %= buffer_size;
    }

    T &operator[](ptrdiff_t pos)
    {
        return *(buffer_start + (begin_shift + pos) % buffer_size);
    }
    const T &operator[](ptrdiff_t pos) const
    {
        return *(buffer_start + (begin_shift + pos) % buffer_size);
    }

    iterator begin()
    {
        return iterator(buffer_start, buffer_size, 0, begin_shift);
    }
    iterator end()
    {
        return iterator(buffer_start, buffer_size, 0, end_shift);
    }

    const_iterator begin() const
    {
        return const_iterator(buffer_start, buffer_size, 0, begin_shift);

    }
    const_iterator end() const
    {
        return const_iterator(buffer_start, buffer_size, 0, end_shift);
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }
    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    ptrdiff_t size() const
    {
        return _size;
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
private:
    void ensure_capacity(ptrdiff_t new_size)
    {
        if (new_size >= buffer_size) {
            T *new_data = reinterpret_cast<T *>(new char[(new_size * 2) * sizeof(T)]);
            for (ptrdiff_t i = 0; i < _size; ++i) {
                new(new_data + i)T((*this)[i]);
            }
            clear();
            delete[]  reinterpret_cast<char *>(buffer_start);
            buffer_start = new_data;
            buffer_size = new_size * 2;
            begin_shift = 0;
            end_shift = _size;
        }
    }

    void inc_end_shift()
    {
        end_shift++;
        end_shift %= buffer_size;
    }
private:
    T *buffer_start;
    ptrdiff_t buffer_size;
    ptrdiff_t begin_shift;
    ptrdiff_t end_shift;
    ptrdiff_t _size;
};

template<typename T>
void swap(circular_buffer<T> &a, circular_buffer<T> &b)
{
    a.swap(b);
}

#endif //EXAM_CIRCULAR_BUFFER_H
