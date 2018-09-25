#include "../common/Helper/RoutingTableHelper.h"
#include "../model/DummySocket.h"
#include "StackCreationHandler.h"
#include "factory/EndpointFactory.h"
#include "factory/StackFactory.h"
#include "fastInspection/FlowIdGenerator.h"

StackCreationHandler::StackCreationHandler(VsObjectFactory &vsObjectFactory, const VirtualStackSettings &settings) :
        _vsObjectFactory(vsObjectFactory),
        _settings(settings),
        _callbackMutex(),
        _dataBuffer(),
        _newStacks(settings.SizeOfStackCreationBuffer, "StackCreationHandler::_newStacks"),
        _southboundCallbacks(std::bind(&StackCreationHandler::handleNewStackResult, this, true, std::placeholders::_1),
                             std::bind(&StackCreationHandler::handleNewStackResult, this, false, std::placeholders::_1)),
        _southboundControl(_southboundCallbacks, vsObjectFactory, settings)
{
}

bool StackCreationHandler::hasNewStacks() const
{
    return _newStacks.available();
}

std::unique_ptr<StackCreationResult> StackCreationHandler::getNextNewStack()
{
    //no need to lock _callbackMutex as only one thread reads _newStacks but many write _newStacks
    //swap storagebuffer here instead of handleNewStackResult, so it happens in the same thread as ::add and we wont loose storages
    auto item = _newStacks.pop();

    adaptStackRequest(*item);
    handleFlowIdAssigment(*item);
    handleRouterSpecifics(*item);

    if (_dataBuffer.find(item->flowId) != _dataBuffer.end())
    {
        item->storageBuffer = std::move(_dataBuffer[item->flowId]);
        _dataBuffer.erase(item->flowId);
    }


    return item;
}

void StackCreationHandler::add(flowid_t flowid, StoragePoolPtr&& storage)
{
    auto bufferIt = _dataBuffer.find(flowid);
    if (bufferIt != _dataBuffer.end())
    {
        if (!bufferIt->second->isFull())
            bufferIt->second->push(std::move(storage));
        //discard storage if buffer is full
        return;
    }

    //std::pair<std::unique_ptr<IEndpoint>, RingBufferMove<StoragePoolPtr>>.
    auto inspectionStruct = InspectionStruct::getInspectionStruct(*storage);

    sockaddr_storage source{};
    sockaddr_storage northboundDestination{};

    //Hole die source und destination aus dem storage
    std::tie(source, northboundDestination) = NetworkExtensions::getSourceAndDestAddr(*inspectionStruct, *storage);

    auto newStackRequest = _southboundControl.createNewStackRequest(northboundDestination,
                                                                    NetworkExtensions::getPort(source),
                                                                    NetworkExtensions::getPort(northboundDestination),
                                                                    inspectionStruct->northboundTransportProtocol,
                                                                    _settings.DefaultStack);
    if (_settings.DefaultStack == StackEnum::SoftwareLoop)
    {

        auto newStackResult = std::make_unique<NewStackResult>(NetworkMessageEnum::NewStack, ConfigurationState::Ok,
                                                               std::make_unique<DummySocket>(),
                                                               northboundDestination,
                                                               std::move(newStackRequest));
        handleNewStackResult(true, std::move(newStackResult));
    }
    else
    {
        auto nextHop = northboundDestination;
        RoutingTableHelper::reRoute(nextHop, _settings);

//        Logger::Log(Logger::DEBUG, _settings.getDefaultIPv4(), "-> Flow: ", inspectionStruct->flowId, " Send to: ", NetworkExtensions::getAddress(northboundDestination), " over: ", NetworkExtensions::getAddress(nextHop));
        _southboundControl.requestCreateStack(nextHop, std::move(newStackRequest));
    }

    //create storageBuffer and add storage
    auto storageBuffer = std::make_unique<RingBufferMove<StoragePoolPtr>>(_settings.SizeOfStackBuffer,
                                                                          "StackCreation_" + std::string(flowid));
    storageBuffer->push(std::move(storage));

    _dataBuffer.emplace(flowid, std::move(storageBuffer));
}

std::future <ConfigurationState> StackCreationHandler::add(const IEndpoint &endpoint, StackEnum stack, const std::string& nextHopIp)
{
//    Logger::Log(Logger::DEBUG, _settings.getDefaultSendIPv4(), "-> Flow: ", endpoint.flowId, " Send to: ",
//                NetworkExtensions::getAddress(endpoint.getNorthboundDestinationAddr()), " over: ",
//                NetworkExtensions::getAddress(nextHop));

    //No need to create a storageBuffer as subsequent requests may create a storageBuffer and the content gets equaly distributed by the stackEngines scheduler

    //source and dest port have to be switched as this represents the
    auto newStackRequest = _southboundControl.createNewStackRequest(endpoint.getNorthboundDestinationAddr(),
                                                                    endpoint.getNorthboundSourcePort(),
                                                                    endpoint.getNorthboundDestinationPort(),
                                                                    endpoint.protocolEnum,
                                                                    stack,
                                                                    &endpoint.partnerFlowId);

    if (stack == StackEnum::SoftwareLoop)
    {
        std::promise<ConfigurationState> loopPromise{};
        loopPromise.set_value(ConfigurationState::Ok);

        auto newStackResult = std::make_unique<NewStackResult>(NetworkMessageEnum::NewStack, ConfigurationState::Ok,
                                                               std::make_unique<DummySocket>(),
                                                               endpoint.getNorthboundDestinationAddr(),
                                                               std::move(newStackRequest));
        handleNewStackResult(true, std::move(newStackResult));
        return loopPromise.get_future();
    } else
    {
        //we get an endpoint, so the stackEngine exists already
        auto nextHop = nextHopIp.empty() ? endpoint.getNorthboundDestinationAddr() : NetworkExtensions::getIPv4SockAddr(nextHopIp, 0);
        RoutingTableHelper::reRoute(nextHop, _settings);

        return _southboundControl.requestCreateStack(nextHop,
                                                     std::move(newStackRequest));
    }
}

void StackCreationHandler::handleNewStackResult(bool byRequest, std::unique_ptr<NewStackResult> result)
{
    if (!result->isValid())
    {
        Logger::Log(Logger::WARNING, "Received failed NewStackResult: ", ConfigurationStateWrapper::toString(result->configurationState));
        return;
    }

    auto stack = StackFactory::createStack(std::move(result->socket), result->request->stack, _settings, _vsObjectFactory);

    std::lock_guard<std::mutex> lock(_callbackMutex);
    _newStacks.push(std::make_unique<StackCreationResult>(byRequest, std::move(stack), nullptr, std::move(result)));
}

void StackCreationHandler::adaptStackRequest(StackCreationResult &result)
{
    auto &request = *result.newStackResult->request;
    if (!request.routed)
    {
        request.origin = request.overrideOriginFlowId ? NetworkExtensions::getIPv4SockAddr(
                static_cast<uint32_t>(request.customOriginFlowId), 0) : result.newStackResult->partner;
    }

    if(!result.byRequest)
    {
        //als receiver:
        //  sourcePort und destPort sind vertauscht aus sicht des receivers
        //  request.origin = public IP des sender, request.destination ist unserer public ip
        //
        std::swap(request.destination, request.origin);
        std::swap(request.northboundDestinationPort, request.northboundSourcePort);
    }

    //overwrite holepunch port for destination with the correct northboundDestionationPort
    NetworkExtensions::setPort(request.destination, request.northboundDestinationPort);
}

void StackCreationHandler::handleFlowIdAssigment(StackCreationResult &result)
{
    auto &request = *result.newStackResult->request;

    //flowId of the source -> target
    auto sourceToTargetFlowId = FlowIdGenerator::createFlowIdForRouter(request.northboundProtocol,
                                                                       request.destination, request.origin,
                                                                       request.northboundSourcePort,
                                                                       request.northboundDestinationPort);

    //flowId of target -> source
    auto targetToSourceFlowId = FlowIdGenerator::createFlowIdForRouter(request.northboundProtocol,
                                                                       request.origin, request.destination,
                                                                       request.northboundDestinationPort,
                                                                       request.northboundSourcePort);

    if(!_settings.IsRouter)
    {
        //We dont caputure the source of any incoming packet from northbound, so set it to 0
        sourceToTargetFlowId.source = 0;
        targetToSourceFlowId.source = 0;

        if(result.byRequest) //if by request, get the public ip of the requester from the receiver so we can calculate the flowId the receiver will calculate
            targetToSourceFlowId.destination = FlowIdGenerator::getIp(result.newStackResult->sourceByPartner);
    }

    result.flowId = sourceToTargetFlowId;
    result.partnerFlowId = targetToSourceFlowId;

    //Change flowId only for clients and not for routers


//    Logger::Log(Logger::DEBUG, "Source: ", _settings.getDefaultSendIPv4(),
//                ", Destination: ", NetworkExtensions::getAddress(request.origin),
//                ", byRequest: ", result.byRequest,
//                ", sourceId: ", result.flowId,
//                ", partnerId: ", result.partnerFlowId,
//                ", withOverride: ", request.overrideOriginFlowId);
}

void StackCreationHandler::handleRouterSpecifics(StackCreationResult& result)
{
    if (!_settings.IsRouter)
        return;

    auto& request = *result.newStackResult->request;
    //we initiated the connection, so we dont have to create a new one
    if (result.byRequest || request.overrideOriginFlowId)
    {
        request.northboundProtocol = TransportProtocolEnum::ROUTE;
        return;
    }

    auto newRequest = request.copy();
    newRequest->routed = true;

    std::swap(newRequest->destination, newRequest->origin);
    std::swap(newRequest->northboundDestinationPort, newRequest->northboundSourcePort);

    if(_settings.RouterOutStack != StackEnum::Invalid)
        newRequest->stack = _settings.RouterOutStack;

    //we are a router, so change the northboundProtocol into Route
    request.northboundProtocol = TransportProtocolEnum::ROUTE;

    auto nextHop = newRequest->destination;
    RoutingTableHelper::reRoute(nextHop, _settings);

    _southboundControl.requestCreateStack(nextHop, std::move(newRequest));

    auto storageBuffer = std::make_unique<RingBufferMove<StoragePoolPtr>>(_settings.SizeOfStackBuffer, "StackCreation_" + std::string(result.partnerFlowId));
    _dataBuffer.emplace(result.partnerFlowId, std::move(storageBuffer));
}
