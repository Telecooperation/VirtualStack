#include "../common/Helper/Debouncer.h"
#include "../common/Helper/NetworkExtensions.h"
#include "../virtualStack/fastInspection/FlowIdGenerator.h"
#include "NetworkMessageController.h"
#include "../VirtualStackSettings.h"
#include "../common/Helper/RoutingTableHelper.h"
#include "../common/Helper/DomainExtensions.h"
#include "../kernel/StreamKernel.h"

NetworkMessageController::NetworkMessageController(const SouthboundCallbacks& callbacks,
												   VsObjectFactory& vsObjectFactory,
												   const VirtualStackSettings& settings)
		: _pendingConfigurationLock(),
		  _callbacks(callbacks),
		  _vsObjectFactory(vsObjectFactory),
		  _settings(settings),
		  _currentState(ConfigurationState::NotInitialized),
		  _pendingConfigurations(settings.SizeOfManagementConfigurationsBuffer),
		  _stopThread(false)
{}

NetworkMessageController::~NetworkMessageController()
{
	stop();
}

void NetworkMessageController::start()
{
	if(_stopThread)
		return;
	
	_thread = std::make_unique<std::thread>(&NetworkMessageController::loop, this);
}

void NetworkMessageController::stop()
{
	_stopThread = true;

	if(_thread && _thread->joinable())
    {
        _thread->join();
        _thread.reset();
    }
}

void NetworkMessageController::loop()
{
    auto ipv4Socket = createTcpIpv4Socket();

	if(!ipv4Socket)
	{
		Logger::Log(Logger::ERROR, "Creation of managementListener sockets failed. Abort listening for managementSockets");
		return;
	}

	auto configResult = configureAsAcceptSocket(ipv4Socket);
	if(!configResult)
	{
		Logger::Log(Logger::ERROR, "Configuration of managementListener sockets failed. Abort listening for managementSockets");
		return;
	}

    StreamKernel ipv4Kernel{std::move(ipv4Socket), _settings, _vsObjectFactory};

	_currentState = ConfigurationState::Ok;

    auto sockEpoll = _vsObjectFactory.socketFactory->createEpoll(*ipv4Kernel.getUnderlyingSocket());
	if (!sockEpoll->isValid())
	{
		_currentState = ConfigurationState::EPollAddFailed;
		return;
	}

    Debouncer debouncer{_settings.NetworkMessageControllerUtilizationPlan, _settings};
	while(!_stopThread)
	{
		processConfigurationBuffer();
		//HINT: for tcp the listen changes the socket to a passive socket and so dataAvailable per FNIoread might not work
		//if (ipv4Kernel.isValid() && ipv4Kernel.dataAvailable())

        if (sockEpoll->waitNonBlocking() == ISocketEpoll::EpollResult::DataAvailable)
			handleIncomingManagementConnections(ipv4Kernel);
        else if (_flowIdToSessionController.empty()) //[else if] instead of [if] so we only sleep if nothing is to do
        {
            debouncer.sleep();
            continue;
        }

        bool didSomeWork = false;
		for( auto it = _flowIdToSessionController.begin(); it != _flowIdToSessionController.end(); ) {
			if (!it->second->isValid())
            {
//                Logger::Log(Logger::DEBUG, "DESTROY: ", it->first);
                it = _flowIdToSessionController.erase(it);
                continue;
            }

            didSomeWork |= it->second->process();
			++it;
		}

        if (didSomeWork)
            debouncer.reset();
        else
            debouncer.sleep();
	}

	//Logger::Log(Logger::DEBUG, "Stopping SessionControllers");
	for(auto& el : _flowIdToSessionController)
	{
		el.second->stop();
	}
}

std::future<ConfigurationState> NetworkMessageController::addConfiguration(const sockaddr_storage& destination,
												std::unique_ptr<BaseSouthboundConfiguration> configuration)
{
	//RingBufferMove only supports 1:1 thread synchronization. so we need to lock as we have n:1
	//This lock transforms the access to this ringBuffer to 1:1 instead of n:1

	auto task = std::make_unique<ConfigurationTask>(destination, std::move(configuration));
	auto future = task->promise.get_future();

	std::lock_guard<std::mutex> lock(_pendingConfigurationLock);
	_pendingConfigurations.push(std::move(task));
	
	return future;
}

void NetworkMessageController::processConfigurationBuffer()
{
	if(!_pendingConfigurations.available())
		return;
	
	auto config = _pendingConfigurations.pop();

	auto catchAllFlowId = FlowIdGenerator::generateCatchAllFlowId(config->destination);
//	Logger::Log(Logger::DEBUG, _settings.getDefaultSendIPv4(), "-> CatchAllFlowId: ", catchAllFlowId, " to: ", NetworkExtensions::getAddress(config->destination));
	auto tmpSessionIter = _flowIdToSessionController.find(catchAllFlowId);
	if(tmpSessionIter == _flowIdToSessionController.end())
	{
//		Logger::Log(Logger::DEBUG, "Create Init-Session: ", catchAllFlowId);
		auto tmpNewSession = std::make_unique<SouthboundSessionController>(config->destination, _callbacks, _vsObjectFactory, _settings); //we are the initiator
		tmpSessionIter = _flowIdToSessionController.emplace(catchAllFlowId, std::move(tmpNewSession)).first;
	}
	tmpSessionIter->second->addConfigurationTask(std::move(config));
}

bool NetworkMessageController::handleIncomingManagementConnections(IKernel& kernel)
{
    sockaddr_storage addr{};
    memset(&addr, 0, sizeof(addr));
    auto managementFD = kernel.accept(addr);
    if (!managementFD)
    {
        Logger::Log(Logger::ERROR, "accept() for a new managementChannel initiated by remote failed");
        return false;
    }

    auto catchAllFlowId = FlowIdGenerator::generateCatchAllFlowId(addr);
    auto newSession = std::make_unique<SouthboundSessionController>(addr, _callbacks, _vsObjectFactory, _settings,
                                                                    std::move(managementFD)); //we are the slave
//	Logger::Log(Logger::DEBUG, "Create Slave-Session: ", catchAllFlowId);

    //Wenn bereits eine Session mit der catchAllFlowId besteht, dann hat der initiator diese bereits gelöscht und eine neue angefordert.
    //Daher kann die schon in der map stehende überschrieben werden, wenn alle ausstehenden operationen fertig sind
    //Es müssen nur operationen abgearbeitet werden, die keine weiteren informationen von der gegenstelle mehr brauchen wie z.b. "killStack"
    auto tmpSessionIter = _flowIdToSessionController.find(catchAllFlowId);
    if (tmpSessionIter != _flowIdToSessionController.end())
    {
        //TODO: Handle reconnect from other side. Should stop current session and create new one
//		Logger::Log(Logger::INFO, "Encounter existing and create a new SessionController for catchAllFlowId: ", catchAllFlowId);
        catchAllFlowId = FlowIdGenerator::generateCatchAllSubFlowId(addr, 1);
//      Logger::Log(Logger::INFO, "Bypassed with catchAllSubFlowId: ", catchAllFlowId);
    }

    auto result = _flowIdToSessionController.emplace(catchAllFlowId, std::move(newSession));

    return result.second;
}

UniqueSocket NetworkMessageController::createTcpIpv4Socket()
{
	auto tmpIpv4Storage = NetworkExtensions::getIPv4SockAddr(_settings.ManagementBindAddress, _settings.ManagementPort);
	auto tmpSocket = _vsObjectFactory.socketFactory->createTCPSocket(DomainExtensions::convertFromSystem(tmpIpv4Storage.ss_family));
	if(!tmpSocket)
	{
		_currentState = ConfigurationState::ManagementListenerSocketCreationFailed;
		return nullptr;
	}

	if(!tmpSocket->setSocketReuse())
	{
		_currentState = ConfigurationState::ManagementListenerSocketReuseFailed;
		return nullptr;
	}

	if(!tmpSocket->bindSocket(tmpIpv4Storage))
	{
		_currentState = ConfigurationState::ManagementListenerPortInUse;
		return nullptr;
	}

	return tmpSocket;
}

bool NetworkMessageController::configureAsAcceptSocket(UniqueSocket& socket)
{
	bool tmpState = true;

	if(!_settings.ManagementInterface.value.empty())
		tmpState &= socket->bindToNetworkDevice(_settings.ManagementInterface.value);

	if(!tmpState)
	{
		_currentState = ConfigurationState::ManagementListenerInterfaceBindFailed;
		return false;
	}

	tmpState &= socket->setAsListenSocket();

	if(!tmpState)
	{
		_currentState = ConfigurationState::ManagementListenerSetAsListenFailed;
		return false;
	}

	return true;
}