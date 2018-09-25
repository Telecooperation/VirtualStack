#include "INorthboundDevice.h"

INorthboundDevice::INorthboundDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory) :
		_pool(vsObjectFactory.getStorageSendPool(settings.SizeOfNorthboundPoolBuffer, "NorthboundPool")), _deviceOpened(false)
{}

INorthboundDevice::~INorthboundDevice()
{
	
}

bool INorthboundDevice::isDeviceOpen()
{
	return _deviceOpened;
}

