
#pragma once

#include "../DataStructures/Container/RingBufferScalar.h"
#include "../Helper/Logger.h"
#include "IPool.h"
#include <atomic>
#include <functional>
#include <malloc.h>
#include <memory>
#include <mutex>

template<typename> class pool_unique_ptr;

/// Preallocated memory pool to manage reuseable memory of type T
/// \tparam T Reusable objects of type T this pool shall manage
template <typename T>
class Pool final : public IPool
{
public:
    /// Create reusable memory pool
    /// \param poolSize Size of (elements - 1) of pool. Size has to multiple of 2
    /// \param debugName Name for logging to display
    /// \param resetFunction Resetfunction to reset objects so they can be reused again
    /// \param args Additional parameters to construct an instance of the containing objects
	template <typename... Arguments>
	Pool(const size_t poolSize, const std::string& debugName, std::function<void(T&)> resetFunction, Arguments&&... args) :
			_mutex(),
			_freeSlots(poolSize, debugName),
			size(_freeSlots.getCapacity()),
			_referenceCounters(new size_t[size]{}),
			_data(static_cast<T*>(malloc(size * sizeof(T)))),
			_resetFunction(resetFunction)
	{
		_referenceCountersPtr = _referenceCounters.get();
		for (size_t i = 0; i < size; ++i)
		{
			new (_data + i) T(std::forward<Arguments>(args)...);
			_referenceCountersPtr[i] = 0;
			_freeSlots.push(i);
		}
	}

    /// Free memory after usage
    /// \param element Element to be freed
	void release(T* element)
    {
        //wenn es nichts mehr gibt hier, muss es auch nicht released werden, da es schon released wurde
        if (_data == nullptr)
            return;

        assert(element != nullptr);
        assert(element >= _data);

        size_t elementIndex = static_cast<size_t>(element - _data);
        assert(elementIndex < size);

        std::lock_guard<std::mutex> lock(_mutex);
        auto tmpOldValue = _referenceCountersPtr[elementIndex];
        if (tmpOldValue == 1)
        {
            --_referenceCountersPtr[elementIndex];
            _freeSlots.push(elementIndex);
        } else if (tmpOldValue == 0)
        {
            _referenceCountersPtr[elementIndex] = 0;
            Logger::Log(Logger::WARNING, "Pool referenceCount overflow. Tried to delete a PoolPtr more times than it was claimed");
        }

        //if this was the last release try to call the lastReleaseCallback if registered
        if (_freeSlots.isFull() && _onLastReleaseCallback)
            _onLastReleaseCallback();
    }

    /// Request an object of type T. If no object is available this method will block
    /// \return Object of pool
	pool_unique_ptr<T> request()
    {
        size_t freeSlot = 0;
        do
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_freeSlots.available())
            {
                freeSlot = _freeSlots.pop();
                ++_referenceCountersPtr[freeSlot];
                break;
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        } while (true);

        T &element = _data[freeSlot];
        _resetFunction(element);
        return pool_unique_ptr<T>(&element, this);
    }

    /// Register an additional reference for this object, so it will behave like a shared ptr
    /// \param element The element to reference again
    /// \return A reference of the given object
//	pool_unique_ptr<T> claim(T* element)
//	{
//		//wenn es nichts mehr gibt hier, muss es auch nicht released werden, da es schon released wurde
//		if(_data == nullptr)
//			return pool_unique_ptr<T>();
//
//		assert(element != nullptr);
//		assert(element >= _data);
//
//		size_t elementIndex = static_cast<size_t>(element - _data);
//		assert(elementIndex < size);
//
//		std::lock_guard<std::mutex> lock(_mutex);
//		++_referenceCountersPtr[elementIndex];
//
//		return pool_unique_ptr<T>(element, this);
//	}

    /// Check if an object can be requested without waiting
    /// \return True if object is available
	bool canRequest() const
	{
		return _freeSlots.available();
	}

    /// Get the amount of usable objects in use or not in this pool
    /// \return The capacity
    size_t getCapacity() const
    {
        return _freeSlots.getCapacity();
    }

    /// Get the amount of free objects
    /// \return Amount of free objects
    size_t getFreeSlotsCount() const
    {
        return _freeSlots.getElementsAvailableCount();
    }

	~Pool() override {
		_freeSlots.stop();
		for (size_t i = 0; i < size; ++i)
		{
			_data[i].~T();
		}
		free(_data);
		_data = nullptr;
	}

    bool isInUse() const override
    {
        return !_freeSlots.isFull();
    }

    void setOnLastReleaseCallback(const std::function<void()>& function) override
    {
        _onLastReleaseCallback = function;
    }

    bool hasReleaseCallback() override
    {
        return static_cast<bool>(_onLastReleaseCallback);
    }

private:
	std::mutex _mutex;
	RingBufferScalar<size_t> _freeSlots;
	const size_t size;

	std::unique_ptr<size_t[]> _referenceCounters;
	size_t* _referenceCountersPtr;
	T* _data; //ist ein tarray als pointer

	std::function<void(T&)> _resetFunction;
    std::function<void()> _onLastReleaseCallback;
};


