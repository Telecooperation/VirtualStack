#pragma once

#include "../VirtualStackSettings.h"
#include "../common/Allocator/VsObjectFactory.h"
#include "../common/DataStructures/Model/Storage.h"

class INorthboundDevice
{
public:
	INorthboundDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory);
	
	virtual ~INorthboundDevice();
	
	virtual void push(StoragePoolPtr packet) = 0;

	virtual StoragePoolPtr pop() = 0;
	
	virtual bool available() = 0;

	virtual bool isFull() = 0;
	
	virtual bool isDeviceOpen();

	virtual Pool<Storage>& getPool() { return *_pool; }
protected:
	PoolRef _pool;
	bool _deviceOpened;
};


