#ifndef MY_CIRCULAR_BUFFER
#define MY_CIRCULAR_BUFFER
#include <memory>
#include <iterator>
#include <type_traits>
constexpr int INIT_SIZE = 10;
template<typename T>
struct circular_buffer;
template<typename T>
void swap(circular_buffer<T> &b1, circular_buffer<T> &b2) noexcept;
template <typename U>
struct circular_iterator;
template <typename T>
struct circular_buffer
{
public:
	using iterator = circular_iterator<T>;
	using const_iterator = circular_iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	circular_buffer() : buffer(nullptr), capacity(0), head(0), tail(0){}
	circular_buffer(circular_buffer const &other): circular_buffer()
	{
		if (!other.empty())
		{
			for (const_iterator it = other.begin(); it != other.end(); it++) { push_back(T(*it)); }
		}
	}
	circular_buffer& operator=(circular_buffer const &other)
	{
		circular_buffer tmp(other);
		swap(*this, tmp);
		return *this;
	}
	~circular_buffer()
	{
		for (const_iterator it = begin(); it != end(); it++) { (*it).~T(); }
		operator delete(buffer);
	}
	size_t size() const
	{
		if (tail >= head)
		{
			return tail - head;
		}
		else
		{
			return (capacity - head + tail);
		}
	}
	void push_back(T const &elem)
	{
		ensure_capacity();
		new (&buffer[tail++]) T(elem);
		tail %= capacity;
	}
	void push_front(T const &elem)
	{
		ensure_capacity();
		size_t head2 = head;
		if (head2 == 0) { head2 = capacity - 1; }
		else { head2--; }
		new(&buffer[head2]) T(elem);
		head = head2;
	}
	void clear()
	{
		for (const_iterator it = begin(); it != end(); it++) { (*it).~T(); }
		tail = 0;
		head = 0;
	}
	bool empty() const
	{
		return size() == 0;
	}
	T& operator[](const size_t index)
	{
		if (head < tail || capacity - head > index) { return buffer[head + index]; }
		else { return buffer[index - (capacity - head)]; }
	}
	T& operator[](const size_t index) const
	{
		if (head < tail || capacity - head >= index) { return buffer[head + index]; }
		else { return buffer[index - (capacity - head)]; }
	}
	T& front() { return operator[](0); }
	T& back() { return operator[](size() - 1); }
	T& front() const { return operator[](0); }
	T& back() const { return operator[](size() - 1); }
	iterator begin() { return iterator(buffer, 0, head, tail, capacity); } const
	iterator end() { return iterator(buffer, size(), head, tail, capacity); } const
	const_iterator begin() const { return const_iterator(buffer, 0, head, tail, capacity); } const
	const_iterator end() const { return const_iterator(buffer, size(), head, tail, capacity); } const
	reverse_iterator rbegin() { return reverse_iterator(end()); } const
	reverse_iterator rend() { return reverse_iterator(begin()); } const
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); } const
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); } const
	iterator insert(const_iterator pos, T const &value)
	{
		//ensure_capacity();
		if (dist(head, pos.index) + 1 <= dist(pos.index, tail))
		{
			push_front(value);
			for (int i = 0; i < pos - const_iterator(begin()); i++) { std::swap(operator[](i), operator[](i + 1)); }
		}
		else
		{
			push_back(value);
			for (int i = size() - 1; i > pos - const_iterator(begin()); i--) { std::swap(operator[](i), operator[](i - 1)); }
		}
		return iterator(buffer, pos - const_iterator(begin()), head, tail, capacity);
	}
	void pop_front()
	{
		buffer[head].~T();
		head++;
		head %= capacity;
	}
	void pop_back()
	{
		if(tail == 0) { tail = capacity - 1; }
		tail--;
		buffer[tail].~T();
	}
	iterator erase(const_iterator pos)
	{
		if (dist(head, pos.index) <= dist(pos.index, tail) - 1)
		{
			for (size_t i = pos - const_iterator(begin()); i > 0; i--) { std::swap(operator[](i), operator[](i - 1)); }
			pop_front();
		}
		else
		{
			for (size_t i = pos - const_iterator(begin()); i < size() - 1; i++) { std::swap(operator[](i), operator[](i + 1)); }
			pop_back();
		}
		return iterator(buffer, pos - const_iterator(begin()), head, tail, capacity);
	}
	template <typename X>
	friend void swap(circular_buffer<X> &b1, circular_buffer<X> &b2) noexcept;
	friend iterator;
	friend const_iterator;
private:
	void ensure_capacity()
	{
		if (capacity == 0)
		{
			buffer = static_cast<T*>(operator new(sizeof(T) * INIT_SIZE));
			capacity = INIT_SIZE;
			head = tail = 0;
		}
		else
			if (size() == capacity - 1)
			{
				T* new_buffer = static_cast<T*>(operator new(sizeof(T) * 2 * capacity));
				int j = 0;
				try
				{
					for (iterator it = begin(); it != end(); it++) 
					{ 
						new (&new_buffer[j]) T(*it); 
						j++;
					}
					for (iterator it = begin(); it != end(); it++) (*it).~T();
					tail = size();
					head = 0;
					operator delete(buffer);
					buffer = new_buffer;
					capacity *= 2;
				}
				catch (std::runtime_error &e)
				{
					for (int i = 0; i <= j; i++) { new_buffer[i].~T(); }
					operator delete(new_buffer);
				}
			}
	}
	size_t dist(size_t one, size_t two)
	{
		if (head < tail || one <= two) { return two - one; }
		else { return capacity - one + two; }
	}
	T *buffer;
	size_t capacity, head, tail;
};
template<typename X>
void swap(circular_buffer<X> &b1, circular_buffer<X> &b2) noexcept
{
	std::swap(b1.buffer, b2.buffer);
	std::swap(b1.capacity, b2.capacity);
	std::swap(b1.head, b2.head);
	std::swap(b1.tail, b2.tail);
}
template <typename U>
struct circular_iterator
{
	using difference_type = ptrdiff_t;
	using value_type = U;
	using pointer = U*;
	using reference = U&;
	using iterator_category = std::random_access_iterator_tag;
	circular_iterator() : base(nullptr), index(0), head(0), tail(0), capacity(0) {}
	circular_iterator(circular_iterator const &other) : base(other.base), index(other.index), head(other.head), tail(other.tail), capacity(other.capacity) {}
	template <typename V>
	circular_iterator(circular_iterator<V> const &other, typename std::enable_if<std::is_same<V const, U>::value && std::is_const<U>::value>::type* = nullptr) :
		base(other.base), index(other.index), head(other.head), tail(other.tail), capacity(other.capacity) {}
	//bool operator==(circular_iterator const other) const noexcept { return base == other.base && index == other.index; }
	circular_iterator& operator++()
	{
		index++;
		return *this;
	}
	circular_iterator operator++(int)
	{
		circular_iterator tmp = *this;
		++*this;
		return tmp;
	}
	circular_iterator& operator--()
	{
		index--;
		return *this;
	}
	circular_iterator operator--(int)
	{
		circular_iterator tmp = *this;
		--*this;
		return tmp;
	}
	reference operator*() const
	{
		if (head < tail || capacity - head > index) { return base[head + index]; }
		else { return base[index - (capacity - head)]; }
	}
	pointer operator->()
	{
		return &operator*();
	}
	circular_iterator& operator+=(ptrdiff_t n)
	{
		index += n;
		return *this;
	}
	circular_iterator& operator-=(ptrdiff_t n)
	{
		index -= n;
		return *this;
	}
	/*ptrdiff_t operator-(circular_iterator &other)
	{
		return index - other.index;
	}*/
	friend circular_buffer<U>;
	friend circular_buffer<typename std::remove_const_t<U>>;
	friend circular_iterator<const U>;
	template <typename T>
	friend circular_iterator<T> operator+(circular_iterator<T> it, ptrdiff_t n);
	template <typename T>
	friend circular_iterator<T> operator+(ptrdiff_t n, circular_iterator<T> it);
	template <typename T>
	friend circular_iterator<T> operator-(circular_iterator<T> it, ptrdiff_t n);
	template <typename T>
	friend circular_iterator<T> operator-(ptrdiff_t n, circular_iterator<T> it);
	template <typename X, typename Y>
	friend bool operator==(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept;
	template <typename X, typename Y>
	friend bool operator<(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept;
	template <typename X, typename Y>
	friend bool operator>(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept;
	template <typename X, typename Y>
	friend bool operator<=(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept;
	template <typename X, typename Y>
	friend bool operator>=(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept;
	template <typename X, typename Y>
	friend bool operator!=(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept;
	/*template <typename U1, typename V1>
	friend std::enable_if_t<std::is_same_v<std::remove_const_t<U1>, std::remove_const_t<V1>>, ptrdiff_t> operator-(circular_iterator<U1> it1, circular_iterator<V1> it2);*/
	template<typename X>
	friend ptrdiff_t operator-(circular_iterator<X> it1, circular_iterator<X> it2);

private:
	U* base;
	size_t index, head, tail, capacity;
	circular_iterator<U>(U* buf, size_t ind, size_t head, size_t tail, size_t capacity) : base(buf), index(ind), head(head), tail(tail), capacity(capacity) {}
};
template <typename U>
circular_iterator<U> operator+(circular_iterator<U> it, ptrdiff_t n)
{
	return circular_iterator<U>(it.base, it.index + n, it.head, it.tail, it.capacity);
}

template <typename U>
circular_iterator<U> operator+(ptrdiff_t n, circular_iterator<U> it)
{
	return circular_iterator<U>(it.base, it.index + n, it.head, it.tail, it.capacity);
}

template <typename U>
circular_iterator<U> operator-(circular_iterator<U> it, ptrdiff_t n)
{
	return circular_iterator<U>(it.base, it.index - n, it.head, it.tail, it.capacity);
}

template <typename U>
circular_iterator<U> operator-(ptrdiff_t n, circular_iterator<U> it)
{
	return circular_iterator<U>(it.base, it.index - n, it.head, it.tail, it.capacity);
}

/*template <typename U, typename V>
std::enable_if_t<std::is_same_v<std::remove_const_t<U>, std::remove_const_t<V>>, ptrdiff_t> operator-(circular_iterator<U> it1, circular_iterator<V> it2)
{
	return it2.index - it1.index;
}*/
template <typename U>
ptrdiff_t operator-(circular_iterator<U> it1, circular_iterator<U> it2) { return it1.index - it2.index; }
template <typename X, typename Y>
bool operator==(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept { return it1.base == it2.base && it1.index == it2.index; }
template <typename X, typename Y>
bool operator<(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept { return it1.index < it2.index; }
template <typename X, typename Y>
bool operator>(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept { return it1.index > it2.index; }
template <typename X, typename Y>
bool operator<=(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept { return it1.index <= it2.index; }
template <typename X, typename Y>
bool operator>=(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept { return it1.index >= it2.index; }
template <typename X, typename Y>
bool operator!=(circular_iterator<X> const it1, circular_iterator<Y> const it2) noexcept { return it1.index != it2.index; }
#endif
