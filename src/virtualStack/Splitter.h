#pragma once

#include "../common/DataStructures/Container/RingBufferMove.h"
#include "../common/DataStructures/Model/Storage.h"

class Splitter final
{
public:
	static bool dispatchToDeepInspection(RingBufferMove<StoragePoolPtr>& inspectionEvents, StoragePoolPtr& storage);
};


