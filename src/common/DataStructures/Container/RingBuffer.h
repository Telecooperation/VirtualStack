
#pragma once

#include "RingBufferBase.h"

/// A threadsafe ringbuffer for one producer, one consumer and unique_ptr elements
/// \tparam T The type of an element
template <typename T>
class RingBuffer final : public RingBufferBase<std::unique_ptr<T>>
{
public:
    /// Create a ringbuffer
    /// \param size The count of elements which has to be a multiple of 2. The final size will be size - 1
    /// \param debugName The name for log messages
	explicit RingBuffer(size_t size, std::string debugName = "") : RingBufferBase<std::unique_ptr<T>>(size, debugName) {}

    /// Add an element into the ringbuffer. This call blocks until there is free space for the element
    /// \param element The element to add
	void push(std::unique_ptr<T> element)
	{
		//Konvention: push darf immer nur vom gleichen Thread aufgerufen werden ( std::this_thread::get_id())
		//check if exceed limits -> wait until free
		while(this->isFull())
		{
			if (this->_stopped.load(std::memory_order_relaxed))
				return;

			//Reicht das als wartezeit bereits aus?
			Logger::Log(Logger::WARNING, "RingBuffer::push: no free slot, Name: ", this->DebugName);
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}

        this->_data[this->upperBoundary.load(std::memory_order_release) & this->internal_capacity()] = std::move(element);
        this->upperBoundary.fetch_add(1, std::memory_order_release);
	}

    /// Get an element of the ringbuffer. This call blocks until there is a element to get
    /// \return The element in a unique_ptr
	std::unique_ptr<T> pop()
	{
		while(this->internal_available() == 0)
		{
			if (this->_stopped.load(std::memory_order_relaxed))
				return std::unique_ptr<T>();

			//Reicht das als wartezeit bereits aus?
			Logger::Log(Logger::WARNING, "RingBuffer::pop: is empty, Name: ", this->DebugName);
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		}

        assert(this->lowerBoundary < this->upperBoundary);

        auto tmpReturn = std::move(this->_data[this->lowerBoundary.load(std::memory_order_release) & this->internal_capacity()]);

        this->lowerBoundary.fetch_add(1, std::memory_order_release);
        return tmpReturn;
	}
};
