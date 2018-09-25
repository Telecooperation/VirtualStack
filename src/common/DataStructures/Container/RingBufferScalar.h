#pragma once

#include "RingBufferBase.h"

/// A threadsafe ringbuffer for one producer, one consumer and copyable elements
/// \tparam T The type of an element
template <typename T>
class RingBufferScalar final : public RingBufferBase<T>
{
public:
    /// Create a ringbuffer
    /// \param size The count of elements which has to be a multiple of 2. The final size will be size - 1
    /// \param debugName The name for log messages
    explicit RingBufferScalar(size_t size, std::string debugName = "") : RingBufferBase<T>(size, debugName) {}

    /// Add an element into the ringbuffer. This call blocks until there is free space for the element
    /// \param element The element to add
	void push(T element)
	{
		while(this->isFull())
		{
            if (this->_stopped.load(std::memory_order::memory_order_relaxed))
                return;

			Logger::Log(Logger::DEBUG, this->DebugName, "::push::isFull()");
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}

        this->_data[this->upperBoundary.load(std::memory_order::memory_order_release) & this->internal_capacity()] = element;
		this->upperBoundary.fetch_add(1, std::memory_order::memory_order_release);
	}

    /// Get an element of the ringbuffer. This call blocks until there is a element to get
    /// \return The element
	T pop()
	{
		while(this->internal_available() == 0)
		{
            if (this->_stopped.load(std::memory_order::memory_order_relaxed))
                return T();

			Logger::Log(Logger::DEBUG, this->DebugName, "::push::isEmpty()");
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}

		assert(this->lowerBoundary < this->upperBoundary);
		auto tmpReturn = this->_data[this->lowerBoundary.load(std::memory_order::memory_order_release) & this->internal_capacity()];
        this->lowerBoundary.fetch_add(1, std::memory_order::memory_order_release);
		return tmpReturn;
	}
};
