#pragma once


#include "../../../interface/INorthboundDevice.h"
#include "../../../common/DataStructures/Container/unique_fd.h"
#include "../../../VirtualStackSettings.h"

#include <poll.h>


//###################
// Alternative: https://github.com/HPENetworking/libviface
//###################
class TunDevice : public INorthboundDevice
{
public:
	TunDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory);
	
	virtual void push(StoragePoolPtr packet) override;
	
	virtual StoragePoolPtr pop() override;
	
	bool available() override;

	bool isFull() override;

	std::string getTunDeviceName(){ return _tunDeviceName; }
private:
	unique_fd allocateTunDevice(const std::string& tunDeviceName, int flags);
	unique_fd _tunDevice;
	pollfd _tunPollFd;
	std::string _tunDeviceName;
};


