#pragma once


#include "../common/DataStructures/VS/IStack.h"
#include "../VirtualStackSettings.h"

class LoopStack final : public IStack
{
public:
	LoopStack(const VirtualStackSettings& settings);
	
	~LoopStack() override;

    void stop() override;
private:
    void start() override;
	void loop();
	
	std::atomic<bool> _threadRunning;
	std::unique_ptr<std::thread> _thread;
};


