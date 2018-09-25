#pragma once


#include "../../VirtualStackSettings.h"
#include "../../common/DataStructures/Model/Storage.h"
#include "../../model/InspectionStruct.h"

class PacketFilter
{
public:
	static bool filterOutIPv6(const VirtualStackSettings& settings, const InspectionStruct& inspectionStruct);
	static bool filterOutRawProtocol(const InspectionStruct& inspectionStruct);
	static bool filterOut(const VirtualStackSettings& settings, const InspectionStruct& inspectionStruct);

};


