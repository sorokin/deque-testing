#ifndef RINGBUFFER_RINGBUFFER_H
#define RINGBUFFER_RINGBUFFER_H

#include<vector>
#include<iostream>
#include<cassert>
#include <cstring>
#include <exception>

template<typename T> class RingBuffer {
private:
    enum endFlags { LEFT, RIGHT, MID, BOTH };

    uint64_t max_size;
    uint64_t cur_size;
    uint64_t begin_id, end_id;
    T* buffer;

    void increst_capacity() {
        uint64_t new_max_size = max_size << 1;
        T* new_buffer = (T*)malloc(sizeof(T)*new_max_size);
        uint64_t cur = 0;

        if (begin_id <= end_id) {
            memcpy(&new_buffer[begin_id], &buffer[begin_id], (end_id - begin_id + 1) * sizeof(T));
        }
        else {
            memcpy(new_buffer, buffer, (end_id + 1) * sizeof(T));
            uint64_t right_size = max_size - begin_id;
            memcpy(&new_buffer[new_max_size - right_size], &buffer[max_size - right_size], (right_size) * sizeof(T));

            begin_id += max_size;
        }

        delete buffer;
        buffer = new_buffer;
        max_size = new_max_size;
    }

    template <typename U>
    struct Iterator {
    private:
        const RingBuffer * buffer;
        uint64_t id;
        endFlags end_flag;
    public:
        typedef Iterator self_type;
        typedef U value_type;
        typedef value_type& reference;
        typedef value_type* pointer;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef int64_t difference_type;

        Iterator(const RingBuffer* bufer, uint64_t id) : buffer(bufer), id(id), end_flag(MID) {}
        Iterator(const RingBuffer* bufer, endFlags end_flag) : buffer(bufer), id(0), end_flag(end_flag) {}

        template <typename V>
        Iterator(const Iterator<V>& other, typename std::enable_if<std::is_same<U, const V>::value>::type* = nullptr) {
            buffer = other.buffer;
            id = other.id;
            end_flag = other.end_flag;
        }

        uint64_t get_id() const { return id; }

        self_type& operator++() {
            switch (end_flag)
            {
                case LEFT:
                    id = buffer->begin_id;
                    end_flag = MID;
                    break;
                case MID:
                    if (id == buffer->end_id) end_flag = RIGHT;
                    id++;
                    id %= buffer->max_size;
            }
            return (*this);
        }
        self_type operator++(int) {
            self_type result(*this);
            ++(*this);
            return result;
        }

        self_type& operator--() {
            switch (end_flag)
            {
                case RIGHT:
                    id = buffer->end_id;
                    end_flag = MID;
                    break;
                case MID:
                    if (id == buffer->begin_id) end_flag = LEFT;
                    if (id == 0) id = buffer->max_size;
                    id--;
            }
            return (*this);
        }
        self_type operator--(int) {
            self_type result(*this);
            --(*this);
            return result;
        }

        self_type operator + (const int add) const {
            if (add == 0) return *this;

            // begin_id < cur_id < end_id
            uint64_t max_size = buffer->max_size;
            uint64_t begin_id = buffer->begin_id + max_size;
            uint64_t cur_id = id + max_size;
            uint64_t end_id = buffer->end_id + max_size;
            if (end_id < begin_id) end_id += max_size;

            if (add < 0) return ((int64_t)cur_id + add >= (int64_t)begin_id) ? self_type(buffer, (cur_id + add) % max_size) : self_type(buffer, LEFT);
            return				((int64_t)cur_id + add <= (int64_t)end_id) ? self_type(buffer, (cur_id + add) % max_size) : self_type(buffer, RIGHT);

        }

        bool operator == (const Iterator other) const {
            if (buffer != other.buffer) return false;
            if (end_flag != other.end_flag) return false;
            if (end_flag != MID) return true;
            return id == other.id;
        }
        bool operator != (const Iterator other) const { return !((*this) == other); }

        reference operator * () const {
            return buffer->buffer[id];
        }
        pointer operator -> () const {
            return &buffer->buffer[id];
        }
    };
public:
    size_t size() const { return cur_size; }
    size_t capacity() const { return max_size; }

    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() { return empty() ? iterator(this, BOTH) : iterator(this, begin_id); }
    const_iterator begin() const { return empty() ? const_iterator(this, BOTH) : const_iterator(this, begin_id); }
    iterator end() { return empty() ? iterator(this, BOTH) : iterator(this, RIGHT); }
    const_iterator end() const { return empty() ? const_iterator(this, BOTH) : const_iterator(this, RIGHT); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    inline bool empty() const {
        return cur_size == 0;
    }
    void clear() {
        cur_size = 0;
        begin_id = 0;
        end_id = max_size - 1;
    }

    RingBuffer(const uint64_t max_size = 1) :max_size(max_size) {
        buffer = (T*)malloc(sizeof(T)*max_size);
        clear();
    }
    RingBuffer(const RingBuffer& other) {
        max_size = other.max_size;
        cur_size = other.cur_size;
        begin_id = other.begin_id;
        end_id = other.begin_id;

        buffer = (T*)malloc(sizeof(T)*max_size);
        memcpy(buffer, other.buffer, sizeof(T)*max_size);
    }
    ~RingBuffer() {
        clear();
    }

    void push_back(const T &element) {
        if (cur_size == max_size) increst_capacity();
        cur_size++;
        end_id++;
        if (end_id == max_size) end_id = 0;
        buffer[end_id] = element;
    }
    void push_front(const T &element) {
        if (cur_size == max_size) increst_capacity();
        cur_size++;
        if (begin_id == 0) begin_id = max_size;
        begin_id--;
        buffer[begin_id] = element;
    }
    void insert(const const_iterator pos, const T &element) {
        if (cur_size == max_size) increst_capacity();

        // begin_id < cur_id < end_id
        uint64_t begin_id = this->begin_id + max_size;
        uint64_t cur_id = pos.get_id() + max_size;
        uint64_t end_id = this->end_id + max_size;
        if (end_id < begin_id) end_id += max_size;

        if (cur_id - begin_id < end_id - cur_id) { //from left

            if (this->begin_id == 0) this->begin_id = max_size;
            this->begin_id--;

            for (uint64_t cur_el = begin_id; cur_el <= cur_id; cur_el++)
                buffer[(cur_el - 1) % max_size] = buffer[cur_el % max_size];
        }
        else { //from right

            this->end_id++;
            if (this->end_id == max_size) this->end_id = 0;

            for (uint64_t cur_el = end_id; cur_el >= cur_id; cur_el--)
                buffer[(cur_el + 1) % max_size] = buffer[(cur_el) % max_size];
        }
        cur_size++;
        buffer[cur_id % max_size] = element;
    }
    void insert(const size_t id, const T &element) { insert(const_iterator(this, (begin_id + id) % max_size), element); }

    void pop_back() {
        assert (cur_size != 0);
        cur_size--;
        if (end_id == 0) end_id = max_size;
        end_id--;
    }
    void pop_front() {
        assert (cur_size != 0);
        cur_size--;
        begin_id++;
        if (begin_id == max_size) begin_id = 0;
    }
    void erase(const const_iterator pos) {
        uint64_t begin_id = this->begin_id + max_size;
        uint64_t cur_id = pos.get_id();
        uint64_t end_id = this->end_id + max_size;
        if (end_id < begin_id) end_id += max_size;
        // begin_id < cur_id < end_id

        if (cur_id - begin_id < end_id - cur_id) {//from left
            pop_front();
            for (uint64_t cur_el = cur_id; cur_el > begin_id; cur_el--)
                buffer[cur_el % max_size] = buffer[(cur_el - 1) % max_size];
        }
        else {//from right
            pop_back();
            for (uint64_t cur_el = cur_id; cur_el < end_id; cur_el++)
                buffer[cur_el % max_size] = buffer[(cur_el + 1) % max_size];
        }
    }
    void erase(uint64_t id) { erase(const_iterator(this, (begin_id + id) % max_size)); }

    T& operator[](uint64_t id) {
        assert (id <= cur_size);
        return buffer[(begin_id + id) % max_size];
    }
    const T operator[](uint64_t id) const { return (*this)[id]; }
    void swap(RingBuffer<T>&right) {
        std::swap(max_size, right.max_size);
        std::swap(cur_size, right.cur_size);
        std::swap(begin_id, right.begin_id);
        std::swap(end_id, right.end_id);
        std::swap(buffer, right.buffer);
    }

    T* front() const { return &buffer[begin_id]; }
    T* back() const { return &buffer[end_id]; }

    void print() {
        printf("%llu/%llu: %llu-->%llu\n", cur_size, max_size, begin_id, end_id);
        for (uint64_t i = 0; i < max_size; i++) {
            printf("%llu: ", i);
            std::cout << buffer[i];
            printf("\n");
        }
    }
};
template<typename T> void swap(RingBuffer<T>&left, RingBuffer<T>&right) {
    left.swap(right);
}

class Tester {
public:
    RingBuffer<int> *buffer;

    Tester() {
        buffer = new RingBuffer<int>(2);
    }

    void print() {
        buffer->print();
        //printf("capacity: %d / %d", buffer->size(), buffer->capacity());
        printf("( --> ) "); for (auto cur = buffer->begin(); cur != buffer->end(); cur++)   printf("%d ", *cur); printf("\n");
        printf("( <-- ) "); for (auto cur = buffer->rbegin(); cur != buffer->rend(); cur++) printf("%d ", *cur); printf("\n");
    }

    void startListen() {
        int element;
        int cmd;
        int id;

        while (true) {
            std::cin >> cmd;
            try {
                switch (cmd) {
                    case 0:
                        std::cin >> id;
                        printf("out: %d\n", (*buffer)[id]);
                        break;
                    case 1:
                        std::cin >> element;
                        buffer->push_front(element);
                        break;
                    case 2:
                        std::cin >> id;
                        std::cin >> element;
                        buffer->insert(id, element);
                        break;
                    case 3:
                        std::cin >> element;
                        buffer->push_back(element);
                        break;
                    case 4:
                        buffer->pop_front();
                        break;
                    case 5:
                        std::cin >> id;
                        buffer->erase(id);
                        break;
                    case 6:
                        buffer->pop_back();
                        break;
                    default:
                        return;
                }
                print();
            }
            catch (std::exception e) {
                printf("error: %s\n", e.what());
            }
        }
    }
};
#endif //RINGBUFFER_RINGBUFFER_H
