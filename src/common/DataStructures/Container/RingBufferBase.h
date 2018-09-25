
#pragma once

#include "../../Helper/ClassMacros.h"
#include "../../Helper/Logger.h"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <thread>

/// A threadsafe ringbuffer for one producer, one consumer. Its a baseclass without push/pop
/// \tparam T The containing type of the buffer
template <typename T>
class RingBufferBase
{
public:
    /// Get the count of available elements to read
    /// \return The count of available elements to read
	size_t getElementsAvailableCount() const
	{
		return internal_available();
	}

    /// Get the count of elements that can be pushed until full
    /// \return The count of available elements to be pushable
    size_t getElementsPushableCount() const
    {
        return internal_capacity() - internal_available();
    }

    /// Get the capacity of this buffer for storage. The capacity is based on size only
    /// \return The capacity tzhis buffer can store at max
	size_t getCapacity() const
	{
		return internal_capacity();
	}

    /// Check if any elements can be read from the buffer
    /// \return True if a elemtent can be read
	bool available() const
	{
        assert(internal_available() <= internal_capacity());
		return internal_available() > 0;
	}

    /// Check if no elements can be added anymore
    /// \return True if no elements can be added
	bool isFull() const
	{
		return internal_available() == internal_capacity();
	}

    /// Stop this ringbuffer, so blocking calls to push/pop can be aborted
	void stop()
	{
		_stopped = true;
	}

    /// Reset this ringbuffer and delete all data in it. Its pristine afterwards
	void reset()
	{
		lowerBoundary = 0;
		upperBoundary = 0;

		_data.reset(new T[_size]{});
	}

    ALLOW_MOVE_SEMANTICS_ONLY(RingBufferBase);

protected:
    /// Create a ringbuffer
    /// \param size The count of elements which has to be a multiple of 2. The final size will be size - 1
    /// \param debugName The name for log messages
	explicit RingBufferBase(size_t size, std::string debugName = "") :
			lowerBoundary(0),
			upperBoundary(0),
			_size(size),
			_data(new T[size]{}),
            _stopped(false),
            DebugName(debugName)
	{
		assert(size >= 2);//Has to have at least 2 elements

		//prüft ob size eine zweierpotenz ist: wenn size eine zweierpotenz ist, ist nur ein bit auf 1 gesetzt,
		//size - 1 hat dann alle bits unter diesem auf 1 und dieses auf 0T, und-verknüpft ergibt das 0, bei nicht-zweierpotenzen
		//ist min. noch ein bit gesetzt, das beim dekrementieren erhalten wird.
		//0 muss extra geprüft werden, da size vorzeichenlos ist
		assert(IS_NUMBER_POWER_OF_2(size));
	}

	virtual ~RingBufferBase()
	{

	}

    /// Get the count of available elements to read
    /// \return The count of available elements to read
	inline size_t internal_available() const
	{
		assert(upperBoundary >= lowerBoundary);
        assert((upperBoundary - lowerBoundary) <= internal_capacity());
		return upperBoundary.load(std::memory_order::memory_order_relaxed) - lowerBoundary.load(std::memory_order::memory_order_relaxed);
	}

    /// Get the capacity of this buffer for storage. The capacity is based on size only
    /// \return The capacity tzhis buffer can store at max
	inline size_t internal_capacity() const
	{
		return _size - 1;
	}

    /// The upper and lower boundary represent borders of used and unused space in the buffer
    /// All between lower and upper boundary is data and both values are absolute values
	std::atomic<size_t> lowerBoundary;
	std::atomic<size_t> upperBoundary;
	const size_t _size;
	std::unique_ptr<T[]> _data;
    std::atomic<bool> _stopped;
    const std::string DebugName;
};
