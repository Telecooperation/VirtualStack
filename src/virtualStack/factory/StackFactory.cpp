#include "../../kernel/DatagramKernel.h"
#include "../../kernel/StreamKernel.h"
#include "../../stacks/GenericStack.h"
#include "../../stacks/LoopStack.h"
#include "../../stacks/updPlus/UdpPlusStack.h"
#include "StackFactory.h"
#include <netinet/ip6.h>


std::unique_ptr<IStack> StackFactory::createStack(UniqueSocket socket,
                                                  const StackEnum stack,
                                                  const VirtualStackSettings &settings,
                                                  VsObjectFactory &vsObjectFactory)
{
    //nach dem southbound suchen
    //endpoint infos -Y damit dann über southbound ein inetworkernel bauen
    //hier eine inetworkkernel generieren lassen davon
    //dann die endpoint infos an den inetworkkernel binden

    auto baseStack = StackEnumHelper::getInfo(stack).BaseStack;
    switch (baseStack)
    {
        case StackEnum::SoftwareLoop:
            return std::make_unique<LoopStack>(settings);
        case StackEnum::TCPIPv4:
            return createStackGeneric<GenericStack, StreamKernel>(stack, std::move(socket), settings, vsObjectFactory);
        case StackEnum::UDPIPv4:
        case StackEnum::UDPLITEIPv4:
        case StackEnum::DCCPIPv4:
        case StackEnum::SCTPIPv4:
            return createStackGeneric<GenericStack, DatagramKernel>(stack, std::move(socket), settings,
                                                                    vsObjectFactory);
        case StackEnum::UDPPlusIPv4:
            return createStackGeneric<UdpPlusStack, DatagramKernel>(stack, std::move(socket), settings,
                                                                    vsObjectFactory);
        default:
        {
            Logger::Log(Logger::ERROR, "No implementation for Stack was found: ",
                        StackEnumHelper::toString(stack));
            return nullptr;
        }
    }
}

bool StackFactory::isSouthboundDeviceValid(const std::string &southboundDeviceName,
                                           const VirtualStackSettings &settings)
{
    auto &devices = settings.SouthboundInterfaces.value;

    if (std::find(devices.begin(), devices.end(), southboundDeviceName) == devices.end())
    {
        Logger::Log(Logger::ERROR, "StackFactory: isSouthboundDeviceValid() -> Unknown southbound device: ",
                    southboundDeviceName);
        return false;
    }
    return true;
}

size_t StackFactory::getOptimalStorageSize(const VirtualStackSettings &settings)
{
    auto &tmpMtuList = settings.SouthboundInterfacesMTU.value;
    if (tmpMtuList.size() == 0)
    {
        Logger::Log(Logger::ERROR, "Failed to calc maxPayloadSize, no southboundinterfaceMtus");
        return 0;
    }

    size_t minMtu = static_cast<size_t>(*std::min_element(tmpMtuList.begin(), tmpMtuList.end()));
    return minMtu;
}

size_t StackFactory::getALMHeaderSize()
{
    /*sequenceNumberMiddleware and payloadSize of StreamKernel*/
    return sizeof(size_t) * 2;
}

size_t StackFactory::getProtocolHeaderSize(const VirtualStackSettings &settings)
{
    size_t tmpResult = sizeof(ip6_hdr);

    tmpResult += static_cast<size_t>(std::max_element(StackEnumInfos.begin(), StackEnumInfos.end(), [](const StackEnumInfo& left, const StackEnumInfo& right) { return left.HeaderSize < right.HeaderSize; })->HeaderSize);

    return tmpResult;
}

template<typename TStack, typename TKernel>
std::unique_ptr<TStack> StackFactory::createStackGeneric(const StackEnum stack,
                                                         UniqueSocket &&socket,
                                                         const VirtualStackSettings &settings,
                                                         VsObjectFactory &vsObjectFactory)
{
    std::unique_ptr<IKernel> kernel(new TKernel(std::move(socket), settings, vsObjectFactory));
    return std::make_unique<TStack>(stack, std::move(kernel), settings, vsObjectFactory);
}
