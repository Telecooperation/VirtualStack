#pragma once

#include "../../Helper/ClassMacros.h"
#include "../../Helper/Logger.h"
#include "../../Helper/make_unique.h"
#include <cassert>
#include <cstdint>
#include <cstring>

/**
 * A fast RAII container for an uint8_t array. No boundary checks will be provided for the purpose of speed
 */
template <typename TModel>
class GenericStorage
{
public:
	/**
	 * Construct a new Storage instance. Creates a new uint8_t[] of size "size". Sets the following: endIndex=size, startIndex=startndex
	 * @param size The size of this Storage instance
	 * @param startIndex The startIndex for data that will be added. The startIndex wont change the size of the internal array. Its only an Index.
	 */
	explicit GenericStorage(size_t size, size_t startIndex = 0) : _endIndex(size), _startIndex(startIndex),
														   _initialSize(size), _initialStartIndex(startIndex)
	{
		_content = std::unique_ptr<TModel[]>(new TModel[size]{});
	}

	/**
	 * Creates a new Storage out of an existing Storage by copy. Allows that the new Storage will be bigger and free space will be prepended
	 * @param content The content to copy from
	 * @param prependFreeSpace The free space before "content" for this new Storage. The size of this Storage will be content.size() + prependFreeSpace
	 */
	explicit GenericStorage(const GenericStorage<TModel>& content, size_t prependFreeSpace = 0) : GenericStorage<TModel>(content.size() + prependFreeSpace, prependFreeSpace)
	{
		copyAfterStart(content.constData(), content.size());
	}

	/**
	 * Create a new Storage out of an initialize list. Will have the size of the initialize_list
	 * @param content The initializer list
	 */
	explicit GenericStorage(std::initializer_list<TModel> content, size_t prependFreeSpace = 0) : GenericStorage<TModel>(content.size() + prependFreeSpace, prependFreeSpace)
	{
		copyAfterStart(content.begin(), content.size());
	}

	/**
	 * Virtually shrinks the storage by increasing the startIndex by numberOfElements
	 * @param numberOfElements The amount the startIndex will be increased
	 */
	void incrementStartIndex(size_t numberOfElements)
	{
		assert((_startIndex + numberOfElements) <= _endIndex);
		assert((_startIndex + numberOfElements) <= _initialSize);
		_startIndex += numberOfElements;
	}
	/**
	 * Virtually increases the storage by decreasing the startIndex by numberOfElements
	 * @param numberOfElements The amount the startIndex will be decreased
	 */
	void decrementStartIndex(size_t numberOfElements)
	{
		assert((static_cast<ssize_t>(_startIndex) - static_cast<ssize_t>(numberOfElements)) >= 0);
		_startIndex -= numberOfElements;
	}

	/**
	 * Moves the endIndex negative wise by numberOfElements. Will virtually shrink the size of the Storage
	 * @param numberOfElements The amount the startIndex will be moved
	 */
	void decrementEndIndex(size_t numberOfElements)
	{
		assert((static_cast<ssize_t>(_endIndex) - static_cast<ssize_t>(numberOfElements)) >= 0);
		assert(_endIndex >= _startIndex);
		_endIndex -= numberOfElements;
	}

	/**
	 * Sets the startIndex to endIndex based on the current endIndex.
	 */
//	void RemoveAllAfterStart()
//	{
//		_startIndex = _endIndex;
//	}

	/**
	 * Sets the endIndex to startIndex based on the current startIndex.
	 */
	void setEndIndexToStartIndex()
	{
		_endIndex = _startIndex;
	}

	/// Sets the virtual size, the size between start and endindex, by changing the endindex
	/// \param size The virtual size of this storage
	void setSize(size_t size)
	{
		assert((_startIndex + size) <= _initialSize);
		_endIndex = _startIndex + size;
	}

	/**
	 * Set the startIndex
	 */
	void setStartIndex(size_t startIndex)
	{
		assert(startIndex <= _endIndex);
		_startIndex = startIndex;
	}

    /// Increment the start index and if the endIndex is smaller, increments it with the startindex
    /// \param numElements Number of elements to increment
	void incrementStartIndexSafe(size_t numElements)
	{
		assert((_startIndex + numElements) < _initialSize);

		_startIndex += numElements;
		_endIndex = std::max(_endIndex, _startIndex);
	}

    /// Decrement the endindex and if the startIndex is bigger, decrements it with the endIndex
    /// \param numElements Number of elements to decrement
	void decrementEndIndexSafe(size_t numElements)
	{
		assert(numElements <= _endIndex);

		_endIndex -= numElements;
		_startIndex = std::min(_endIndex, _startIndex);
	}

	/**
	 * Prepends data with size of contentSize to this Storage. Decreases the startIndex by contentSize
	 * @param content The data to prepend
	 * @param contentSize The size of the Data. It can be less then the actual size of content
	 */
	template <typename T>
	void prependDataBeforeStart(const T *content, size_t contentSize)
	{
		size_t tmpStructSizeInVectorSize = contentSize / GenericStorage<TModel>::getStorageDataTypeSize();
		assert(_startIndex >= tmpStructSizeInVectorSize);
		_startIndex -= tmpStructSizeInVectorSize;
		std::memcpy(data(), content, tmpStructSizeInVectorSize);
	}

	/**
	 * Prepends data with size of sizeof(T) to this Storage. Decreases the startIndex by sizeof(T)
	 * @param content The data to prepend
	 */
	template <typename T>
	void prependDataAutomaticBeforeStart(const T *content)
	{
		size_t contentSize = sizeof(T);
		prependDataBeforeStart(content, contentSize);
	}

	/**
	 * Prepends data with size of sizeof(T) to this Storage. Decreases the startIndex by sizeof(T)
	 * @param content The data to prepend
	 */
	template <typename T>
	void prependDataScalarBeforeStart(const T content)
	{
		size_t contentSize = sizeof(T);
		prependDataBeforeStart(&content, contentSize);
	}

    /**
     * Appends data with size of contentSize to this Storage at endIndex. Increases the endIndex by contentSize
     * @param content The data to append
     * @param contentSize The size of the Data. It can be less then the actual size of content
     */
    template <typename T>
    void appendDataAfterEnd(const T *content, size_t contentSize)
    {
        size_t tmpStructSizeInVectorSize = contentSize / GenericStorage<TModel>::getStorageDataTypeSize();

        assert((_endIndex + tmpStructSizeInVectorSize) <= _initialSize);
        std::memcpy(_content.get() + _endIndex, content, tmpStructSizeInVectorSize);
        _endIndex += tmpStructSizeInVectorSize;
    }

	/**
	 * Appends a Storage with size of contentSize to this Storage at endIndex. Increases the endIndex by contentSize
	 * @param content The Storage to append its data
	 * @param contentSize The size of the Data. It can be less then the actual size of content
	 */
	void appendDataAfterEnd(const GenericStorage<TModel> &content, size_t contentSize)
	{
		assert((_endIndex + contentSize) <= _initialSize);
		std::memmove(_content.get() + _endIndex, content.constData(), contentSize);
		_endIndex += contentSize;
	}


	/**
	  * Appends a Storage with size of contentSize to this Storage at endIndex. Increases the endIndex by contentSize
	 * @param content The Storage to append its data
	 * @param contentSize The size of the Data. It can be less then the actual size of content
	 * @param contentStartOffset Offset to the startIndex of source
	 */
	void appendDataAfterEnd(const GenericStorage<TModel> &content, size_t contentSize, size_t contentStartOffset)
	{
		assert((_endIndex + contentSize) <= _initialSize);
		std::memmove(_content.get() + _endIndex, content.constData() + contentStartOffset, contentSize);
		_endIndex += contentSize;
	}

	/**
	 * Appends data with size of sizeof(T) to this Storage at endIndex. Increases the endIndex by sizeof(T)
	 * @param content The data to append
	 */
	template <typename T>
	void appendDataAutomaticAfterEnd(const T *content)
	{
		size_t contentSize = sizeof(T);
		size_t tmpStructSizeInVectorSize = contentSize / GenericStorage<TModel>::getStorageDataTypeSize();

		assert(_startIndex <= _endIndex);
		assert((_startIndex) <= _initialSize);
		assert((_endIndex + tmpStructSizeInVectorSize) <= _initialSize);

		std::memcpy(_content.get() + _endIndex, content, tmpStructSizeInVectorSize);

		_endIndex += tmpStructSizeInVectorSize;
	}

	/**
	 * Appends data with size of sizeof(T) to this Storage at endIndex. Increases the endIndex by sizeof(T)
	 * @param content The data to append
	 */
	template <typename T>
	void appendDataScalarAfterEnd(const T content)
	{
		size_t contentSize = sizeof(T);
		appendDataAfterEnd(&content, contentSize);
	}

    /**
	 * Appends data with size of sizeof(T) to this Storage at endIndex. Increases the endIndex by sizeof(T)
	 * @param content The data to append
	 */
    void appendDataScalarAfterEnd(const std::string& content)
    {
        appendDataAfterEnd(content.data(), content.size());
    }

    /**
	 * Replaces data with size of sizeof(T) in this Storage at endIndex - position - sizeof(T). Make sure: startIndex <= (endIndex - position - sizeof(T))
	 * @param content The data to replace
     * @param position The position after the data is to be replaced
	 */
    template <typename T>
    void replaceDataScalarBeforeEnd(const T content, size_t position = 0)
    {
        size_t contentSize = sizeof(T);
        size_t tSize = contentSize / GenericStorage<TModel>::getStorageDataTypeSize();

        assert(_endIndex >= (position + sizeof(T)));
        assert(_startIndex <= (_endIndex - position - sizeof(T)));

        std::memcpy(_content.get() + _endIndex - position - tSize, &content, tSize);
    }

    /**
	 * Converts a struct T into an Storage. The struct will be copied. Respects little-endian byte order
	 * @param header The struct to be converted
	 * @return The struct as Storage
	 */
    template <typename T>
    static inline GenericStorage<TModel> toStorage(T header)
    {
        size_t tmpTypeSize = sizeof(T) / sizeof(TModel);
        GenericStorage<TModel> result(tmpTypeSize);
        result.copyAfterStart(reinterpret_cast<uint8_t *>(&header), tmpTypeSize);
        return result;
    }
    /**
     * Converts a Storage into a struct T. The data will be copied. Respects little-endian byte order
     * @param contentSize The size of the new struct (because you maybe want to fill out only the first values and not all of the struct)
     * @param startIndex The startIndex for the content to look for the data
     * @return A instance of T with the data from Storage
     */
    template <typename T>
    inline T toType(size_t contentSize, size_t startIndex = 0) const
    {
        if((size() - startIndex) < contentSize)
        {
            Logger::Log(Logger::ERROR, "Tried to convert a StorageToType, but the ContentSize greater then content.size() - startIndex [startIndex was too high]");
            return T();
        }

        T result;
        std::memset(&result, 0, sizeof(T)); //alles auf 0 setzen, wegen contentSize und startIndex: man könnte nur die mitte von T beschreiben
        auto resultPtr = reinterpret_cast<uint8_t*>(&result); //wegen littleendian system
        std::memcpy(resultPtr, constData() + startIndex, contentSize);
        return result;
    }

    /**
     * Converts a Storage into a struct T. The data will be copied. Respects little-endian byte order. Will use sizeof(T) for the contentSize
     * @param startIndex The startIndex for the content to look for the data
     * @return A instance of T with the data from Storage
     */
    template <typename T>
    inline T toTypeAutomatic(size_t startIndex = 0) const
    {
        size_t contentSize = sizeof(T);
        if((size() - startIndex) < contentSize)
        {
            Logger::Log(Logger::ERROR, "Tried to convert a StorageToType, but the ContentSize greater then content.size() - startIndex [startIndex was too high]");
            return T();
        }

        T result;
        std::memset(&result, 0, sizeof(T)); //alles auf 0 setzen, wegen contentSize und startIndex: man könnte nur die mitte von T beschreiben
        auto resultPtr = reinterpret_cast<uint8_t*>(&result); //wegen littleendian system
        std::memcpy(resultPtr, constData() + startIndex, contentSize);
        return result;
    }

	/**
	 * Copies data into this Storage by replacing. The size of the Storage will remain unchanged
	 * @param source The data to insert
	 * @param size The size of the data to be inserted
	 * @param destinationStartIndex A relative index at which the data should be inserted, based on the current index of this Storage
	 */
	void copyAfterStart(const TModel *source, size_t size, size_t destinationStartIndex = 0)
	{
		//assert((_startIndex + destinationStartIndex + size) <= _endIndex);
		//assert((_startIndex + destinationStartIndex + size) <= _initialSize);
		//std::memcpy(data() + destinationStartIndex, source, size);

		assert((_startIndex + destinationStartIndex + size) <= _initialSize);
		std::memcpy(data() + destinationStartIndex, source, size);
		_endIndex = std::max(_startIndex + destinationStartIndex + size, _endIndex);
	}

    /**
     * Moves the data from the current startIndex to the initialStartIndex and sets the indices correctly
     */
    void moveDataToInitialStartIndex()
    {
        //dont do anything if the current startIndex is lessequals the initialStartIndex. data is to the uppermost left
        if(_startIndex <= _initialStartIndex)
            return;

        //attention: memcpy doenst work if memory is overlapping
        if(_startIndex != _endIndex)
            memmove(_content.get() + _initialStartIndex, data(), size());

        _endIndex = _initialStartIndex + size();
        _startIndex = _initialStartIndex;
    }

	/**
	 * Replace this Storage instance by the given one by moving its data into this instance
	 * @param content The content for replacing. Its data will be moved into this instance
	 */
	void replaceWith(GenericStorage<TModel> &content)
	{
		_startIndex = content._startIndex;
		_endIndex = content._endIndex;
		_initialStartIndex = content._initialStartIndex;
		_initialSize = content._initialSize;
		_content.swap(content._content);
	}

	/**
	 * Fill the underlying array completely with 0s
	 */
	void nullifyData()
	{
		memset(_content.get(), 0, _initialSize);
	}

	/**
	 * A pointer to the internal data array based on the startIndex
	 * @return The data array as pointer
	 */
	TModel* data(size_t startIndex)
	{
		return _content.get() + startIndex;
	}

	/**
	 * A pointer to the internal data array based on the startIndex
	 * @return The data array as pointer
	 */
	TModel* data()
	{
		return _content.get() + _startIndex;
	}

	/**
	 * A const pointer to the internal data array based on the startIndex
	 * @return The data array as a const pointer
	 */
	const TModel* constData() const
	{
		return _content.get() + _startIndex;
	}

	/**
	 * A const pointer to the internal data array based on the startIndex
	 * @return The data array as a const pointer
	 */
	const TModel* constData(size_t startIndex) const
	{
		return _content.get() + startIndex;
	}

	/**
	 * The virtual size of the data based on the startIndex
	 * @return The virtual size of the data
	 */
	size_t size() const
	{
		assert((static_cast<ssize_t>(_endIndex) - static_cast<ssize_t>(_startIndex)) >= 0);
		return _endIndex - _startIndex;
	}

	/**
	 * Set the startIndex of this instance
	 * @param value The new startIndex
	 */
//	void setStartIndex(size_t value)
//	{
//		_startIndex = value;
//	}

	/**
	 * Get the intial startIndex set on creation of this instance
	 * @return The initial startIndex
	 */
	size_t getInitialStartIndex() const
	{
		return _initialStartIndex;
	}

	size_t getStartIndex() const
	{
		return _startIndex;

	}

	size_t getInitialSize() const
	{
		return _initialSize;
	}

	/**
	 * Resets the initial default values
	 */
	void reset()
	{
		_startIndex = _initialStartIndex;
		_endIndex = _initialStartIndex;
	}

    /// Sets the startIndex to 0 and the endIndex to initialSize
	void fullyExpandIndices()
	{
		_startIndex = 0;
		_endIndex = _initialSize;
	}

    /// Sets the endIndex to initialSize
	void expandEndIndex()
	{
		_endIndex = _initialSize;
	}

	/**
	 * Resets to the startIndex to the initial startIndex
	 */
	void resetToInitialStartIndex()
	{
		_startIndex = _initialStartIndex;
	}

	/**
	 * Get the size of free space left ahead of the data
	 * @return Size of free space ahead of data
	 */
	size_t freeSpaceForPrepend() const
	{
		return _startIndex;
	}
	
	
	/**
	 * Get the size of free space left ahead of the data
	 * @return Size of free space ahead of data
	 */
	size_t freeSpaceLeftAfterStart() const
	{
		return _initialSize - _startIndex;
	}

	/**
	 * Get the size of free space left after the data
	 * @return Size of free space after data
	 */
	size_t freeSpaceForAppend() const
	{
		assert((static_cast<ssize_t>(_initialSize) - static_cast<ssize_t>(_endIndex)) >= 0);
		return _initialSize - _endIndex;
	}

	/**
	 * Creates of full copy if this instance
	 * @return A copy of this instance
	 */
	GenericStorage<TModel> copy() const
	{
		auto tmpResult = GenericStorage<TModel>(_initialSize);
		tmpResult.copyAfterStart(_content.get(), _initialSize);
		tmpResult._endIndex = _endIndex;
		tmpResult._startIndex = _startIndex;
		tmpResult._initialStartIndex = _initialStartIndex;
		tmpResult._initialSize = _initialSize;
		return tmpResult;
	}
	
	/**
	 * Creates of full copy if this instance
	 */
	void copyInto(GenericStorage<TModel>& storage) const
	{
		assert(_initialSize <= storage._initialSize);
		storage._endIndex = _endIndex;
		storage._startIndex = _startIndex;
		storage._initialStartIndex = _initialStartIndex;

		std::memmove(storage._content.get(), _content.get(), _initialSize);
	}

	/**
	 * Creates of partial copy if this instance, starting at startIndex
	 * @param startIndex The startIndex to start copying into the new instance
	 * @return A partial copy of this instance
	 */
	GenericStorage<TModel> copy(size_t startIndex) const
	{
		assert(startIndex <= _endIndex);
		assert(startIndex <= _initialSize);

		auto tmpStorageSize = _endIndex - startIndex;
		auto tmpResult = GenericStorage<TModel>(tmpStorageSize);

		std::memcpy(tmpResult.data(), constData() + startIndex, tmpStorageSize);
		return tmpResult;
	}

	/**
	 * Creates of partial copy if this instance, starting at startIndex and ending at endIndex
	 * @param startIndex The startIndex to start copying into the new instance
	 * @param numberOfElements The number of elements to copy into new instance
	 * @return A partial copy of this instance
	 */
	GenericStorage<TModel> copy(size_t startIndex, size_t numberOfElements) const
	{
		assert(startIndex <= numberOfElements);
		assert(startIndex <= _endIndex);
		assert(startIndex <= _initialSize);
		assert((static_cast<ssize_t>(numberOfElements) - static_cast<ssize_t>(startIndex)) >= 0);

		auto tmpStorageSize = numberOfElements - startIndex;
		auto tmpResult = GenericStorage<TModel>(tmpStorageSize);
		std::memcpy(tmpResult.data(), constData() + startIndex, tmpStorageSize);
		return tmpResult;
	}

	/**
	 * Gets the data-type size of the internal data-array
	 * @return The data-type size of the internal data-array
	 */
	static size_t getStorageDataTypeSize()
	{
		return sizeof(TModel);
	}

	/**
	 * Creates a Storage which is filled completely with the given value
	 * @param size The size of the Storage
	 * @param value The value this Storage should be filled with
	 * @param startIndex The startIndex of the Storage, same like the startIndex of the constructor
	 */
	static GenericStorage<TModel> createFilledStorage(size_t size, TModel value, size_t startIndex = 0)
	{
		auto tmpStorage = GenericStorage<TModel>(size, startIndex);
		memset(tmpStorage._content.get(), value, size);
		return tmpStorage;
	}

	/**
	 * Creates a normal Storage of size "size" but with endIndex=startIndex so its virtual size=0 and data can be appended to it
	 * @param size The size of this Storage instance
	 * @param startIndex The startIndex for data that will be added. The startIndex wont change the size of the internal array. Its only an Index.
	 */
	static GenericStorage<TModel> createAppendableStorage(size_t size, size_t startIndex = 0)
	{
		auto tmpStorage = GenericStorage<TModel>(size, startIndex);
		tmpStorage._endIndex = startIndex;
		return tmpStorage;
	}

	/**
	 * Creates an Empty Storage Instance will shall be used as "no-data"
	 */
	static GenericStorage<TModel> createEmptyStorage() { return GenericStorage<TModel>(0); }

	/**
	 * Create a normal Storage of size "size" but with startIndex=size so its virtual size=0 and data can be prepended to it
	 * @param size The size of this Storage instance
	 */
	static GenericStorage<TModel> createPrependableStorage(size_t size) { return GenericStorage<TModel>(size, size); }

	inline const TModel& operator[](const size_t nIndex) const
	{
		return constData()[nIndex];
	}

    inline TModel& operator[](const size_t nIndex)
    {
        return data()[nIndex];
    }

    /// Destroys the data in it and resets all indices. The Storage is not usable anymore
	void dispose()
	{
		if(_disposed)
			return;

		_disposed = true;
		_endIndex = 0;
		_startIndex = 0;
		_initialSize = 0;
		_initialStartIndex = 0;
		_content.reset();
	}

    ALLOW_MOVE_SEMANTICS_ONLY(GenericStorage<TModel>);
private:
	size_t _endIndex;
	size_t _startIndex;

	size_t _initialSize;
	size_t _initialStartIndex;

	bool _disposed = false;
	std::unique_ptr<TModel []> _content;
};

template <typename T>
using GenericStorageUniquePtr = std::unique_ptr<GenericStorage<T>>;
