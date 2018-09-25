#pragma once

#include "../../Helper/ClassMacros.h"
#include <initializer_list>
#include <memory>

/// Wraps a unique_ptr array with the size of it
/// \tparam TModel The mode to make an array of
template <typename TModel>
class UniquePtrArray
{
public:
    /// Create an array per initializer list
    /// \param list The elements for the array
	UniquePtrArray(std::initializer_list<std::unique_ptr<TModel>> list) :
			Size(list.size()),
			Elements(new std::unique_ptr<TModel>[list.size()]{}) {
		for (size_t i = 0; i < Size; ++i)
		{
			std::unique_ptr<TModel>* tmpModel = const_cast<std::unique_ptr<TModel>*>(list.begin() + i);
			Elements[i].reset(tmpModel->release());
		}
	}

    /// Create an array with a given size. Initializes the elements with the default constructor
    /// \param size The size of the array
	UniquePtrArray(size_t size) :
			Size(size),
			Elements(new std::unique_ptr<TModel>[size]{}) {	}
	
	TModel* operator[](const size_t nIndex)
	{
		return Elements[nIndex].get();
	}

    /// Get the size of the array
    /// \return The size
	size_t size() { return Size; }

	const size_t Size;
	std::unique_ptr<std::unique_ptr<TModel>[]> Elements;

    ALLOW_MOVE_SEMANTICS_ONLY(UniquePtrArray);
};
