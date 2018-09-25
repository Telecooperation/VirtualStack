#pragma once

#include "../../../VirtualStackSettings.h"
#include "../../../common/DataStructures/Container/RingBufferMove.h"
#include "../../../common/DataStructures/Model/Storage.h"
#include "../../../stacks/StackEnum.h"
#include "../../Helper/ClassMacros.h"
#include <memory>

class IStack
{
public:
	IStack(const StackEnum stackEnum, const VirtualStackSettings& settings) :
			stackInfo(StackEnumHelper::getInfo(stackEnum)),
            _stackId(0),
            _isManagement(false),
			_isConnectionClosed(false),
			_settings(settings),
			_fromStackBuffer(settings.SizeOfStackBuffer, "IStack.cpp(" + StackEnumHelper::toString(stackEnum) + ")::_fromStackBuffer"),
			_toStackBuffer(settings.SizeOfStackBuffer, "IStack.cpp(" + StackEnumHelper::toString(stackEnum) + ")::_toStackBuffer")
	{}
	virtual ~IStack();
	
	void start(size_t stackId, bool isManagement = false);

    virtual void stop();

	bool isConnectionClosed() const {
		return _isConnectionClosed;
	}

    virtual void push(StoragePoolPtr storage);

    virtual StoragePoolPtr pop();

    virtual bool isFull();

    virtual bool available();

    virtual size_t getPushFreeSlots() const;

    virtual size_t getPushSlotsInUse() const;

    size_t getStackId() const;
    bool isManagementStack() const;

	const StackEnumInfo& stackInfo;
	
	ALLOW_MOVE_SEMANTICS_ONLY(IStack);
	uint8_t stackIndex = 0;
protected:
    virtual void start() = 0;
    size_t _stackId;
    bool _isManagement;
	std::atomic<bool> _isConnectionClosed;
	const VirtualStackSettings& _settings;
	RingBufferMove<StoragePoolPtr> _fromStackBuffer;
	RingBufferMove<StoragePoolPtr> _toStackBuffer;
};


