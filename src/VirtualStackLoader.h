
#pragma once


#include "VirtualStack.h"
#include "VirtualStackSettings.h"
#include "common/Allocator/VsObjectFactory.h"
#include "common/DataStructures/VS/SettingsProvider.h"
#include "model/PosixSocketFactory.h"
#include "virtualStack/factory/StackFactory.h"

template <typename TNorthboundInterface, typename TSocketFactory = PosixSocketFactory>
class VirtualStackLoader {
public:
    VirtualStackLoader() {}
    
    bool Initialize(const std::string filename) {
        SettingsProvider settingsProvider;
        if (!settingsProvider.ReadSettings(filename)) {
            Logger::Log(Logger::ERROR, "Settings-Read failed");
            return false;
        }

        auto settings = std::make_unique<VirtualStackSettings>(std::move(settingsProvider));
        if (settings->SettingsReadFailed())
            return false;

        return Initialize(std::move(settings));
    }

    bool Initialize(std::unique_ptr<VirtualStackSettings> settings)
    {
        virtualStackSettings = std::move(settings);

        auto& vsSettings = *virtualStackSettings;

        vsObjectFactory = createVSObjectFactory(vsSettings);

        inspectionEvents.reset( new RingBufferMove<StoragePoolPtr>{vsSettings.SizeOfInspectionEventsBuffer});

        auto dummyNorthDevice = std::make_unique<TNorthboundInterface>(vsSettings, *vsObjectFactory);
        northboundDevice = dummyNorthDevice.get();

        virtualStack.reset(new VirtualStack(std::move(dummyNorthDevice), *vsObjectFactory, *inspectionEvents, vsSettings));
        if(!virtualStack->start())
            return false;

        initialized = true;
        return true;
    }

    static std::unique_ptr<VsObjectFactory> createVSObjectFactory(const VirtualStackSettings& vsSettings)
    {
        auto minMTU = StackFactory::getOptimalStorageSize(vsSettings) + sizeof(InspectionStruct);
        auto protocolHeaderSize = StackFactory::getProtocolHeaderSize(vsSettings) + sizeof(InspectionStruct);
        auto almHeaderSize = StackFactory::getALMHeaderSize();
        auto socketFactory = std::make_unique<TSocketFactory>();
        return std::make_unique<VsObjectFactory>(std::move(socketFactory), minMTU, protocolHeaderSize, almHeaderSize);
    }

    bool initialized = false;
    std::unique_ptr<VirtualStackSettings> virtualStackSettings;
    std::unique_ptr<VsObjectFactory> vsObjectFactory;
    std::unique_ptr<RingBufferMove<StoragePoolPtr>> inspectionEvents;
    std::unique_ptr<VirtualStack> virtualStack;
    TNorthboundInterface* northboundDevice = nullptr;
    
    ALLOW_MOVE_SEMANTICS_ONLY(VirtualStackLoader);
};


