
#pragma once


#include "../common/DataStructures/Container/RingBuffer.h"
#include "../common/DataStructures/Container/unique_fd.h"
#include "../common/DataStructures/Model/Storage.h"
#include "../common/Helper/ClassMacros.h"
#include "../model/InspectionStruct.h"
#include "../model/StackCreationResult.h"
#include "../southbound/SouthboundControl.h"
#include "stackEngine/StackEngine.h"

class StackCreationHandler
{
public:
    StackCreationHandler(VsObjectFactory& vsObjectFactory, const VirtualStackSettings& settings);

    void add(flowid_t flowid, StoragePoolPtr&& storage);
    std::future <ConfigurationState> add(const IEndpoint& endpoint, StackEnum stack, const std::string& nextHopIp = nullptr);
    bool hasNewStacks() const;
    std::unique_ptr<StackCreationResult> getNextNewStack();

    void start() { _southboundControl.start(); }
    void stop() { _southboundControl.stop(); }

    SouthboundControl& getSouthboundControl() { return _southboundControl; }

    ALLOW_MOVE_SEMANTICS_ONLY(StackCreationHandler);
private:
    VsObjectFactory& _vsObjectFactory;
    const VirtualStackSettings& _settings;

    std::mutex _callbackMutex;
    std::map<flowid_t, std::unique_ptr<RingBufferMove<StoragePoolPtr>>> _dataBuffer;
    RingBuffer<StackCreationResult> _newStacks;

    SouthboundCallbacks _southboundCallbacks;
    SouthboundControl _southboundControl;

    void handleNewStackResult(bool byRequest, std::unique_ptr<NewStackResult> result);
    inline void adaptStackRequest(StackCreationResult& result);
    inline void handleFlowIdAssigment(StackCreationResult& result);
    inline void handleRouterSpecifics(StackCreationResult& result);
};

//von northbopund:
/*
 * rufe add() auf:
 * lege RingBuffer an und füge dort den Storage ein. Lege RequestNewStack an und gibs an Southbound. Gib delegat mit, gekapselt mit flowid std::bind(, flowid)
 *
 * delegat:
 * erstelle Stack, Schiebe Stack in RingBuffer für VS-Loop (auch in VS-Receiver registierern) -> VS_Loop legt StackEngine bei bedarf an
 *
 * wenn von southbound neuer Stack:
 * gleicher delegat wie von per northbound initiiert
 *
 * hint:
 * Der Endpoint-Typ steht im NewStackRequest
 */


