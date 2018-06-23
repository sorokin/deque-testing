#include <algorithm>
#include <iterator>

template<typename S>
class circular_buffer;

template<typename S>
class iterator1 {
    template<typename T>
    friend
    class circular_buffer;

    template<typename C>
    friend
    class iterator1;

public:
    typedef std::ptrdiff_t difference_type;
    typedef S value_type;
    typedef S *pointer;
    typedef S &reference;
    typedef std::bidirectional_iterator_tag iterator_category;

private:
    S *data;
    size_t ind, left, capacity;

    iterator1(S *data, size_t ind, size_t left, size_t capacity) : data(data), ind(ind),
                                                                   left(left), capacity(capacity) {}

public:
    template<typename = std::enable_if_t<std::is_const<S>::value>>
    iterator1(const iterator1<std::decay_t<S>> &other) {
        data = other.data;
        ind = other.ind;
        left = other.left;
        capacity = other.capacity;
    }

    reference operator*() const {
        return *(data + ((left + ind) % capacity));
    }

    pointer operator->() const {
        return (data + ((left + ind) % capacity));
    }

    iterator1 &operator++() {
        ++ind;
        return *this;
    }

    iterator1 &operator--() {
        --ind;
        return *this;
    }

    iterator1 operator++(int) {
        iterator1 cur = *this;
        ++ind;
        return cur;
    }

    iterator1 operator--(int) {
        iterator1 cur = *this;
        --ind;
        return cur;
    }

    friend iterator1 operator+(iterator1 const &oth, difference_type add) {
        return iterator1(oth.data, oth.ind + add, oth.left, oth.capacity);
    }

    friend iterator1 operator+(difference_type add, iterator1 const &oth) {
        return iterator1(oth.data, oth.ind + add, oth.left, oth.capacity);
    }

    friend iterator1 operator-(iterator1 const &oth, difference_type add) {
        return iterator1(oth.data, oth.ind - add, oth.left, oth.capacity);
    }

    friend iterator1 operator-(difference_type add, iterator1 const &oth) {
        return iterator1(oth.data, oth.ind - add, oth.left, oth.capacity);
    }

    friend iterator1 operator+=(iterator1 &oth, difference_type add) {
        oth = oth + add;
        return oth;
    }

    friend iterator1 operator-=(iterator1 &oth, difference_type add) {
        oth = oth - add;
        return oth;
    }

    friend difference_type operator-(iterator1 const &first, iterator1 const &second) {
        return first.ind - second.ind;
    }

    template<typename U, typename I>
    friend bool operator==(iterator1<U> first, iterator1<I> second);

    template<typename U, typename I>
    friend bool operator!=(iterator1<U> first, iterator1<I> second);

    bool operator<(iterator1 second) const {
        return ind < second.ind;
    }

    bool operator<=(iterator1 second) const {
        return *this < second || *this == second;
    }

    bool operator>(iterator1 second) const {
        return !(*this <= second);
    }

    bool operator>=(iterator1 second) const {
        return !(*this < second);
    }
};


template<typename T>
class circular_buffer {
    size_t left, size_, capacity;
    T *data;

    void ensure_capacity(size_t size) {
        if (size >= capacity) {
            circular_buffer<T> oth(capacity * 2 + 1);
            for (size_t i = 0, temp = left; i < size_; ++i, temp = (temp + 1) % capacity) {
                oth.push_back(data[temp]);
            }
            swap(*this, oth);
        }
    }


public:

    circular_buffer() : left(0), size_(0), capacity(0), data(nullptr) {}

    circular_buffer(size_t capacity) : left(0), size_(0), capacity(capacity),
                                       data(reinterpret_cast<T *>(new char[sizeof(T) * capacity])) {}


    circular_buffer(const circular_buffer &other) : left(other.left), size_(0),
                                                    capacity(other.capacity), data(nullptr) {
        if (capacity == 0)
            return;
        data = reinterpret_cast<T *>(new char[sizeof(T) * capacity]);
        for (size_t i = 0, temp = left; i < other.size_; ++i, temp = (temp + 1) % capacity) {
            push_back(other.data[temp]);
        }
    }

    ~circular_buffer() {
        clear();
        delete[] reinterpret_cast<char *>(data);
    }

    circular_buffer &operator=(const circular_buffer &other) {
        circular_buffer<T> oth(other);
        swap(*this, oth);
        return *this;
    }

    size_t size() {
        return size_;
    }

    bool empty() {
        return size_ == 0;
    }

    void clear() {
        while (!empty()) {
            pop_back();
        }
    }

    void push_back(const T &dat) {
        ensure_capacity(size_ + 1);
        new(&data[(left + size_) % capacity]) T(dat);
        ++size_;
    }

    void pop_back() {
        data[(left + size_ - 1) % capacity].~T();
        --size_;
    }

    void push_front(const T &dat) {
        ensure_capacity(size_ + 1);
        left = (left - 1 + capacity) % capacity;
        new(&data[left]) T(dat);
        ++size_;
    }

    void pop_front() {
        data[left].~T();
        ++left;
        left %= capacity;
        --size_;
    }

    T &front() {
        return data[left];
    }

    T &front() const {
        return data[left];
    }

    T &back() {
        return data[(left + size_ - 1) % capacity];
    }

    T &back() const {
        return data[(left + size_ - 1) % capacity];
    }

    T &operator[](size_t i) {
        return data[(left + i) % capacity];
    }

    T &operator[](size_t i) const {
        return data[(left + i) % capacity];
    }

    typedef iterator1<T> iterator;
    typedef iterator1<const T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    iterator begin() {
        return iterator(data, 0, left, capacity);
    }

    const_iterator begin() const {
        return const_iterator(data, 0, left, capacity);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    iterator end() {
        return iterator(data, size_, left, capacity);
    }

    const_iterator end() const {
        return const_iterator(data, size_, left, capacity);
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    iterator erase(const_iterator pos) {
        size_t ind = pos.ind;
        iterator cur = iterator(data, ind, left, capacity);
        if (2 * ind <= size_) {
            while (cur != begin()) {
                *cur = *(cur - 1);
                cur--;
            }
            pop_front();
        } else {
            while (cur != --end()) {
                *cur = *(cur + 1);
                cur++;
            }
            pop_back();
        }
        return iterator(data, ind, left, capacity);
    }

    iterator insert(const_iterator pos, const T &dat) {
        size_t ind = pos.ind;
        if (2 * ind <= size_) {
            push_front(dat);
            iterator temp = iterator(data, ind, left, capacity), cur = begin();
            while (cur != temp) {
                *(cur) = *(cur + 1);
                cur++;
            }
            *cur = dat;
        } else {
            push_back(dat);
            iterator temp = iterator(data, ind, left, capacity), cur = end() - 1;
            while (cur != temp) {
                *cur = *(cur - 1);
                cur--;
            }
            *cur = dat;
        }
        return iterator(data, ind, left, capacity);

    }

    template<typename T1>
    friend void swap(circular_buffer<T1> &first, circular_buffer<T1> &second);
};

template<typename T>
void swap(circular_buffer<T> &first, circular_buffer<T> &second) {
    std::swap(first.left, second.left);
    std::swap(first.size_, second.size_);
    std::swap(first.capacity, second.capacity);
    std::swap(first.data, second.data);
}

template<typename U, typename I>
bool operator==(iterator1<U> first, iterator1<I> second) {
    return first.data == second.data && first.ind == second.ind
           && first.left == second.left && first.capacity == second.capacity;
}

template<typename U, typename I>
bool operator!=(iterator1<U> first, iterator1<I> second) {
    return !(first == second);
}
