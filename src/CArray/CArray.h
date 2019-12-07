#ifndef CIRCULAR_ARRAY
#define CIRCULAR_ARRAY

// barebones implementation of a circular array

#include <cassert>

template <typename T, unsigned N>
class CArray
{
    static_assert(N <= 32, "Container meant for small amounts of data");
    static_assert(N!=2 || N!=4 || N!= 8 || N!=16 || N!=32, "Use power of 2 for N");

public:
    CArray() : pos(0) {};
    CArray(const CArray<T,N>&) = default;
    CArray& operator=(const CArray<T,N>&) = default;

    void Enque(const T& val);
    void Enque(T&& val);
    void Deque();

    T& Front();
    const T& Front() const;

    T& operator[](int idx);
    const T& operator[](int idx) const;

private:
    T arr[N];
    unsigned pos;
};

template <typename T, unsigned N>
void CArray<T,N>::Enque(const T& val)
{
    arr[pos] = val;
    pos = (pos+1) & (N-1);
}

template <typename T, unsigned N>
void CArray<T,N>::Enque(T&& val)
{
    arr[pos] = val;
    pos = (pos+1) & (N-1);
}

template <typename T, unsigned N>
void CArray<T,N>::Deque()
{
    arr[pos].~T();
    pos = (pos|N - 1) & (N-1);
}

template <typename T, unsigned N>
T& CArray<T,N>::Front()
{
    return (*this)[pos];
}

template <typename T, unsigned N>
const T& CArray<T,N>::Front() const
{
    return (*this)[pos];
}

template <typename T, unsigned N>
const T& CArray<T,N>::operator[](int i) const
{
    // could work for negative i as well
    assert(i >= 0);
    return arr[((pos|N) - i) & (N-1)];
}

template <typename T, unsigned N>
T& CArray<T,N>::operator[](int i)
{
    const CArray<T,N>* cthis = this;
    return const_cast<T&>((*cthis)[i]);
}

#endif
