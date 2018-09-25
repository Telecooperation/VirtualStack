#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <net/if.h>

#include "TunDevice.h"

TunDevice::TunDevice(const VirtualStackSettings& settings, VsObjectFactory &vsObjectFactory) : INorthboundDevice(settings, vsObjectFactory)
{
	_tunDeviceName = settings.TunDeviceName.value;
	_tunDevice = allocateTunDevice(_tunDeviceName, IFF_TUN | IFF_NO_PI);
	
	_tunPollFd.fd = _tunDevice;
	_tunPollFd.events = POLLIN | POLLPRI | POLLERR;
}

void TunDevice::push(StoragePoolPtr packet)
{
	//give it to tundevice
	//annahme, das packet zeigt auf den richtigen startindex
	//laut POSIX ist write() threadsafe
	auto tmpBytesSent = write(_tunDevice, packet->data(), packet->size());
	if(tmpBytesSent < 0)
		Logger::Log(Logger::ERROR, "Error sending packet to tun device", strerror(errno));
	else if(static_cast<size_t>(tmpBytesSent) != packet->size())
		Logger::Log(Logger::ERROR, "Sending to tunDevice was not complete, bytes to send should be: ", packet->size(), " but only: ", tmpBytesSent, " bytes were sent");
}

StoragePoolPtr TunDevice::pop()
{
	//check if tun device has data, if notz nullptr.
	//get poolptr anbd read data otherwise
	
	StoragePoolPtr tmpStorage = _pool->request();
	//because tmpStorage.size() == 0, because its an appendStorage
	auto tmpBytesRead = read(_tunDevice, tmpStorage->data(), tmpStorage->getInitialSize() - tmpStorage->getStartIndex());
	if (tmpBytesRead < 0) {
		Logger::Log(Logger::ERROR, "Error reading packet from tun device", strerror(errno));
		return StoragePoolPtr();
	}
	
	tmpStorage->setSize(static_cast<size_t>(tmpBytesRead));
	return tmpStorage;
}

unique_fd TunDevice::allocateTunDevice(const std::string& tunDeviceName, int flags)
{
	struct ifreq ifr;
	const char *clonedev = "/dev/net/tun";
	
	/* Arguments taken by the function:
	 *
	 * char *dev: the name of an interface (or '\0'). MUST have enough
	 *   space to hold the interface name if '\0' is passed
	 * int flags: interface flags (eg, IFF_TUN etc.)
	 */
	
	/* open the clone device */
	
	int fd = open(clonedev, O_RDWR);
	if (fd < 0) {
		Logger::Log(Logger::ERROR, "Failed to open the clone device");
		return unique_fd();
	}
	unique_fd result{fd};
	
	/* preparation of the struct ifr, of type "struct ifreq" */
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = static_cast<short>(flags); /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
	
	if (!tunDeviceName.empty()) {
		/* if a device name was specified, put it in the structure; otherwise,
		 * the kernel will try to allocate the "next" device of the
		 * specified type */
		strncpy(ifr.ifr_name, tunDeviceName.c_str(), IFNAMSIZ);
	}
	
	/* try to create the device */
	if (ioctl(fd, TUNSETIFF, static_cast<void*>(&ifr)) < 0) {
		Logger::Log(Logger::ERROR, "Error creating tun device with ioctl: ", strerror(errno));
		return unique_fd();
	}
	
	/* if the operation was successful, write back the name of the
	 * interface to the variable "dev", so the caller can know
	 * it. Note that the caller MUST reserve space in *dev (see calling
	 * code below) */
	_tunDeviceName = std::string(ifr.ifr_name);
	
	Logger::Log(Logger::INFO, "Tun allocation done.");
	_deviceOpened = true;
	/* this is the special file descriptor that the caller will use to talk
	 * with the virtual interface */
	return result;
}

bool TunDevice::available()
{
	int ret = poll(&_tunPollFd,1,0);
	
	if(ret < 0)
	{
		if(errno != EINTR)
			Logger::Log(Logger::ERROR, "Error polling tun device: ", strerror(errno));
		return false;
	}
	
	return ret > 0;
}

bool TunDevice::isFull() {
    return false;
}
