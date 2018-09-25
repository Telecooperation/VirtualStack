#pragma once


#include "../../VirtualStackSettings.h"
#include "../../common/DataStructures/VS/IStack.h"
#include "../../interface/IEndpoint.h"
#include "../../interface/IKernel.h"
#include "../../interface/ISocket.h"
#include "../../stacks/StackEnum.h"
#include <string>

class StackFactory
{
public:
	static std::unique_ptr<IStack>	createStack(UniqueSocket socket, const StackEnum stack,
										   const VirtualStackSettings &settings,
										   VsObjectFactory &vsObjectFactory);

	static size_t getALMHeaderSize();
	static size_t getProtocolHeaderSize(const VirtualStackSettings &settings);

	static size_t getOptimalStorageSize(const VirtualStackSettings &settings);

    template <typename TStack, typename TKernel>
    static std::unique_ptr<TStack> createStackGeneric(const StackEnum stack,
                                                      UniqueSocket&& socket,
                                                      const VirtualStackSettings& settings,
                                                      VsObjectFactory& vsObjectFactory);
private:
	static bool isSouthboundDeviceValid(const std::string& southboundDeviceName, const VirtualStackSettings &settings);

};


