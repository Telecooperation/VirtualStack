#pragma once

#include "../model/StackDetails.h"
#include "../common/DataStructures/Model/Storage.h"
#include "../common/DataStructures/Container/FixedSizeArray.h"

class IScheduler
{
public:
	explicit IScheduler(const FixedSizeArray<StackDetails>& stackDetails);
	
	virtual ~IScheduler();
	virtual void process(const Storage &packet) = 0;

	size_t getActiveStack();
	
	DELETE_COPY_AND_COPY_ASSIGN_CONSTRUCTOR(IScheduler);
protected:
    inline void sanitizeActiveStackIndex()
    {
        if(_activeStackIndex >= _stackDetails.getSize()) //overflow, activate first stack again
            _activeStackIndex = 0;
    }

    inline void skipInactiveStacks()
    {
        for (sanitizeActiveStackIndex(); _stackDetails[_activeStackIndex].isInactive; sanitizeActiveStackIndex())
            ++_activeStackIndex;
    }

    const FixedSizeArray<StackDetails>& _stackDetails;
    size_t _activeStackIndex;
};


