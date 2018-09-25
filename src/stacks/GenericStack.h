#pragma once


#include "../VirtualStackSettings.h"
#include "../common/Allocator/VsObjectFactory.h"
#include "../common/DataStructures/VS/IStack.h"
#include "../common/Helper/Debouncer.h"
#include "../interface/IKernel.h"

class GenericStack final : public IStack
{
public:
	GenericStack(const StackEnum stackType,
                 std::unique_ptr<IKernel> kernel,
				 const VirtualStackSettings& settings,
                 VsObjectFactory& vsObjectFactory);

    ~GenericStack() override;

    void stop() override;
private:
    void start() override;

	std::unique_ptr<IKernel> _kernel;
    IKernel& _kernelPtr;
	
	void loop();

	std::atomic<bool> _threadRunning;
	std::unique_ptr<std::thread> _thread;
};


