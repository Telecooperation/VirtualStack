#pragma once

#include <cstddef>
#include <algorithm>
#include "../../Helper/Logger.h"
#include "../../Helper/make_unique.h"
#include "../../Helper/ClassMacros.h"

template <typename T>
class FixedSizeArray final
{
public:
    explicit FixedSizeArray(size_t size) : capacity(size),
                                         _size(0),
                                         _data(new T[size]{})
    {
    }

    T* add(T&& el)
    {
        if(isFull())
            return nullptr;

        _data[_size] = std::move(el);

        ++_size;

        return &_data[_size - 1];
    }

    T remove(size_t index)
    {
        if(!checkBounds(index))
            return T{};

        //swap removed value with last valid one
        std::swap(_data[index], _data[_size - 1]);

        --_size;

        return std::move(_data[_size]);
    }

    T* get(size_t index)
    {
        if(!checkBounds(index))
            return nullptr;

        return &_data[index];
    }

    const T* get(size_t index) const
    {
        if(!checkBounds(index))
            return nullptr;

        return &_data[index];
    }

    size_t getSize() const
    {
        return _size;
    }

    bool isFull() const
    {
        return _size >= capacity;
    }

    T* getRawData()
    {
        return _data.get();
    }

    void reset()
    {
        _size = 0;
        _data.reset(new T[capacity]{});
    }

    const T& operator[](const size_t index) const
    {
        return _data[index];
    }

    T& operator[](const size_t index)
    {
        return _data[index];
    }

    T* begin() const
    {
        return _data.get();
    }

    T* end() const
    {
        return _data.get() + _size;
    }

    const size_t capacity;

    ALLOW_MOVE_SEMANTICS_ONLY(FixedSizeArray);
private:

    inline bool checkBounds(size_t index) const {
        if (index < _size)
            return true;

        Logger::Log(Logger::ERROR, "IndexedArray: Tried to access data with index out of bounds, index: ", index, ", size: ", _size);
        return false;
    }

    size_t _size;
    std::unique_ptr<T[]> _data;
};
