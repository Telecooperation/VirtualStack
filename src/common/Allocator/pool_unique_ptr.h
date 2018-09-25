
#pragma once

#include "../Helper/ClassMacros.h"
#include "Pool.h"
#include <algorithm>

/// Container for memory requested from a pool
/// \tparam T Type of memory
template <typename T>
class pool_unique_ptr final
{
    friend class Pool<T>;

public:
    /// Default construtor to create a dummy instance with no memory attached
	pool_unique_ptr() : _data(nullptr), _pool(nullptr) { }

    /// Release memory from pool on destruction
    virtual ~pool_unique_ptr() {
        if(_pool != nullptr)
            _pool->release(_data);
        _pool = nullptr;
    }

    /// Get a pointer of the memory
    /// \return Pointer to memory of type T
	T* get()
	{
		return _data;
	}

    /// Free memory from pool and reset this poolptr. It will be a dummy instance afterwards
	void reset()
	{
		if(_pool != nullptr)
		{
			_pool->release(_data);
			_pool = nullptr;
			_data = nullptr;
		}
	}

//	pool_unique_ptr<T> claim() const
//	{
//		if(_pool == nullptr || _data == nullptr)
//			return pool_unique_ptr<T>();

//		return _pool->claim(_data);
//	}

    /// Get a reference of the memory
    /// \return Reference to memory of type T
	T& operator*() const
	{
		return *_data;

	}

    /// Const overload for deref operator
    /// \return Pointer to memory of type T
	T* operator->() const noexcept
	{
		return _data;
	}

    /// Overload for deref operator
    /// \return Pointer to memory of type T
	T* operator->() noexcept
	{
		return _data;
	}

    /// Bool overload.
    /// \return True if this poolptr contains a memory object, False if it is a dummy poolptr containing no memory
	operator bool() const noexcept {
		return _data != nullptr;
	}

	DELETE_COPY_AND_COPY_ASSIGN_CONSTRUCTOR(pool_unique_ptr);
	pool_unique_ptr<T> &operator=(pool_unique_ptr && other) noexcept
	{
        reset();
		_data = other._data;
		_pool = other._pool;

		other._data = nullptr;
		other._pool = nullptr;

		return *this;
	}

	/**
	 * Other must be nullified, because otherwise when it would be destroyed, it would delete the slot, which we claim ownership to.
	 */
	pool_unique_ptr(pool_unique_ptr<T>&& other) noexcept : _data(other._data), _pool(other._pool)   {
		other._data = nullptr;
		other._pool = nullptr;
	}

private:
    pool_unique_ptr(T* data, Pool<T>* pool) : _data(data), _pool(pool) { }
	T* _data;
	Pool<T>* _pool;
};




