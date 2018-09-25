
#pragma once

#include "../../Allocator/pool_unique_ptr.h"
#include "GenericStorage.h"

///Provides all typedefs for a Storage

typedef GenericStorage<uint8_t> Storage;
typedef std::unique_ptr<Storage> StorageUniquePtr;
typedef pool_unique_ptr<Storage> StoragePoolPtr;

