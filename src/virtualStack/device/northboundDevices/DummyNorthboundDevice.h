#pragma once


#include "../../../common/DataStructures/Container/RingBufferMove.h"
#include "../../../interface/INorthboundDevice.h"
#include <mutex>

class DummyNorthboundDevice final : public INorthboundDevice
{
public:
	DummyNorthboundDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory);

	void push(StoragePoolPtr packet) override;
	
	StoragePoolPtr pop() override;
	
	bool available() override;
	
	void externalIntoNorthbound(StoragePoolPtr packet);
	
	StoragePoolPtr externalOutOfNorthbound();
	
	bool canSend();
	
	bool availableExternal();

	bool isFull() override;

	~DummyNorthboundDevice() override;
private:
	//Has to be mutex, as its methods will be called from many StackEngines
	std::mutex _outOfVirtualStackPushMutex;
	RingBufferMove<StoragePoolPtr> _outOfVirtualStackBuffer;
	RingBufferMove<StoragePoolPtr> _intoVirtualStackBuffer;
};