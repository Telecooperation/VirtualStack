#pragma once

#include "../../common/DataStructures/Model/Storage.h"
#include "../../common/Helper/ClassMacros.h"
#include <cstddef>

/**
 * An interface providing methods to store an object of a class into a storage
 */
class ISerializeable
{
public:
	ISerializeable();
	virtual ~ISerializeable();
	
	virtual void serialize(Storage& storage) const = 0;
	virtual void deserialize(const Storage& storage) = 0;
	virtual size_t size() const = 0;
	
	ALLOW_MOVE_SEMANTICS_ONLY(ISerializeable);
};


