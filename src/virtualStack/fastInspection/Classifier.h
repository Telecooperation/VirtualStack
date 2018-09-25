#pragma once


#include "../../model/InspectionStruct.h"

class Classifier final
{
public:
	static InspectionStruct* process(StoragePoolPtr &storagePoolPtr);
};


