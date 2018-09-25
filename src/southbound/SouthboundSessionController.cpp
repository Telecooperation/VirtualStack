#include "../VirtualStackSettings.h"
#include "../common/Allocator/VsObjectFactory.h"
#include "../common/Helper/NetworkExtensions.h"
#include "../virtualStack/factory/StackFactory.h"
#include "SouthboundSessionController.h"
#include "configuration/NewStack/Request/NewStackRequestConfiguration.h"
#include "configuration/NewStack/Respond/NewStackRespondConfiguration.h"

SouthboundSessionController::SouthboundSessionController(const sockaddr_storage &dest,
														 const SouthboundCallbacks& callbacks,
														 VsObjectFactory &vsObjectFactory,
														 const VirtualStackSettings &settings, UniqueSocket managementFd) :
		state(ConfigurationState::NotInitialized),
		_callbacks(callbacks),
		_vsObjectFactory(vsObjectFactory),
		_settings(settings),
		_pool(vsObjectFactory.getStorageSendPool(settings.SizeOfManagementSessionsBuffer, "SouthboundSessionController")),
        _hadOneConfigTask(false),
		_nextSessionId((managementFd == nullptr) ? 0u : 1u), //slave has uneven sessionId numbering
		_sessionIdToConfiguration()
{
    if (managementFd == nullptr)
    {
        //run async task to create managementStack for this sessionController
        _createManagementStackPromise = std::async(std::launch::async,
                                                   &SouthboundSessionController::createManagementStack,
                                                   std::ref(_settings),
                                                   std::ref(_vsObjectFactory),
                                                   dest,
                                                   true);
    }
    else
    {
        state = ConfigurationState::Ok;
        _managementStackDestAddr = dest;
        _managementStack = StackFactory::createStack(std::move(managementFd), StackEnum::TCPIPv4, settings, vsObjectFactory);
		if(_managementStack == nullptr)
		{
            Logger::Log(Logger::ERROR, "SouthboundSessionController -> Settings for Stack invalid: ", StackEnumHelper::toString(StackEnum::TCPIPv4));
			state = ConfigurationState::SettingsForStackInvalid;
			return;
		}

        _managementStack->start(0, true);
    }
}

bool SouthboundSessionController::process() {
    if(state == ConfigurationState::NotInitialized)
    {
        auto resultStatus = _createManagementStackPromise.wait_for(std::chrono::seconds(0));
        if (resultStatus != std::future_status::ready)
            return true; //false would cause the deletion of this sessionController

        std::tie(state, _managementStack, _managementStackDestAddr) = _createManagementStackPromise.get();
        if (!isValid())
        {
            stop();
            return false;
        }
    }

	if (!isValid())
		return false;

    if (_managementStack->isConnectionClosed())
    {
        state = ConfigurationState::ManagementClosed;
        return false;
    }

	if (_managementStack->available())
		processReceivedStorage(_managementStack->pop());

//  Cannot be used unless both sides agree on connection closing per custom data exchange
//	if (_sessionIdToConfiguration.empty())
//    {
//        if(_hadOneConfigTask)
//            state = ConfigurationState::NoConfigurationsClosed;
//        return false;
//    }

	for( auto it = _sessionIdToConfiguration.begin(); it != _sessionIdToConfiguration.end(); ) {
	    auto sessionId = it->first;
	    auto& task = it->second;

	    if(!task->hasStarted()) //start configuration if it has not been started already
            task->configuration->start(_managementStackSourceAddr, _managementStackDestAddr);

		bool oneMessageSent = processSendMessage(sessionId, *task);

		//if no message has been sent and a finish request has been issued, we can accept it, as there is no pending stuff to do
		if (!oneMessageSent && task->configuration->requestsFinish())
			task->configuration->acceptFinish();

		if (task->isFinished())
		{
			it = _sessionIdToConfiguration.erase(it);
			continue;
		}

		++it;
	}

	return true;
}

void SouthboundSessionController::addConfigurationTask(std::unique_ptr<ConfigurationTask> task)
{
	//add config into list
	_sessionIdToConfiguration.emplace(_nextSessionId, std::move(task));
    _hadOneConfigTask = true;
	//increment nextSessionId
	updateNextSessionId();
}

void SouthboundSessionController::processReceivedMessage(std::unique_ptr<NetworkReceiveMessage>&& message)
{
	if(!message)
        return;

	auto configIterator = _sessionIdToConfiguration.find(message->sessionId);
	if(configIterator == _sessionIdToConfiguration.end())
	{
		//sessionId not found, time to add a new configuration based on the type and connect it with the sessionid
		//how to handle sessionId collision?
		
		//the sessionid has to be unique for both sides, initiator and follower.
		auto newConfigTask = createTaskFromReceivedMessage(*message);
		if (!newConfigTask)
            return;

		configIterator = _sessionIdToConfiguration.emplace(message->sessionId, std::move(newConfigTask)).first;
        _hadOneConfigTask = true;
	}
	
	auto& configTask = configIterator->second;
	configTask->configuration->addReceiveFromNetwork(std::move(message));
}

bool SouthboundSessionController::processSendMessage(size_t sessionId, ConfigurationTask& task) {
	auto &tmpConfig = *task.configuration;
    //abort if nothing to send or buffer of stack is full so we cant add the new message
	if (!tmpConfig.hasSendToNetwork())
		return false;
	if (_managementStack->isFull())
		return true;

	auto tmpStorage = _pool->request();

	NetworkSendMessage newMessage{sessionId,
								  tmpConfig.networkMessageEnum,
								  tmpConfig.getSendToNetwork()};
    newMessage.serialize(*tmpStorage);
	_managementStack->push(std::move(tmpStorage));
	return true;
}

void SouthboundSessionController::updateNextSessionId()
{
	_nextSessionId += 2; //immer +2 und nicht +1 weil wir von initiator zu slave fortlaufende sessionIds haben wollen.
	//so ist auch anhand der sessionId erkennbar, ob man initiator oder slave ist.
}

void SouthboundSessionController::stop() {
//	Logger::Log(Logger::DEBUG, "Init SessionController STOP");
    if (state == ConfigurationState::Ok)
	    process(); //one last call to process and if configurations arent done by that, just kill them

//	Logger::Log(Logger::DEBUG, "SessionController STOP -> cancle running configs");
	for( auto& item : _sessionIdToConfiguration)
    {
        item.second->cancel();
        if (!item.second->isFinished())
        {
            Logger::Log(Logger::DEBUG, "Wait for finish Configuration");
            do
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            } while (!item.second->isFinished());
        }
    }

    _managementStack->stop();
    _sessionIdToConfiguration.clear();
    state = ConfigurationState::Canceled;
}

std::tuple<ConfigurationState, std::unique_ptr<IStack>, sockaddr_storage>
    SouthboundSessionController::createManagementStack(const VirtualStackSettings& settings,
                                                       VsObjectFactory& vsObjectFactory,
                                                       sockaddr_storage managementStackDestAddr,
                                                       bool isInitiator)
{
    auto sourceAddress = settings.ManagementBindAddress; //So we bind it to the same as the management to support multihoming
	auto manSocket = vsObjectFactory.socketFactory->createSocketForHolePunching(InternetProtocolEnum::IPv4,
                                                                                TransportProtocolEnum::TCP,
                                                                                sourceAddress);
	if(!manSocket)
	{
        Logger::Log(Logger::ERROR, "SouthboundSessionController -> SessionSocketBindFailed");
		return std::make_tuple(ConfigurationState::SessionSocketBindFailed, nullptr, managementStackDestAddr);
	}

	NetworkExtensions::setPort(managementStackDestAddr, settings.ManagementPort);
	manSocket = vsObjectFactory.socketFactory->holePunching(std::move(manSocket), isInitiator, TransportProtocolEnum::TCP, managementStackDestAddr, true);
	if (!manSocket)
	{
        Logger::Log(Logger::ERROR, "SouthboundSessionController -> SessionHolePunchingFailed");
        return std::make_tuple(ConfigurationState::SessionHolePunchingFailed, nullptr, managementStackDestAddr);
	}

	auto managementStack = StackFactory::createStack(std::move(manSocket), StackEnum::TCPIPv4, settings, vsObjectFactory);
	if(managementStack == nullptr)
    {
        Logger::Log(Logger::ERROR, "SouthboundSessionController -> SettingsForStackInvalid");
        return std::make_tuple(ConfigurationState::SettingsForStackInvalid, nullptr, managementStackDestAddr);
    }

    managementStack->start(0, true);

	return std::make_tuple(ConfigurationState::Ok, std::move(managementStack), managementStackDestAddr);
}

std::unique_ptr<ConfigurationTask> SouthboundSessionController::createTaskFromReceivedMessage(const NetworkReceiveMessage &message)
{
	switch(message.messageType)
	{
		case NetworkMessageEnum::NewStack:
		{
//			Logger::Log(Logger::DEBUG, _settings.getDefaultSendIPv4(), ": GOT NewStack from: ", NetworkExtensions::getAddress(message.source));
			auto configTask = std::make_unique<ConfigurationTask>(message.source,
																  std::make_unique<NewStackRespondConfiguration>(
																		  _callbacks.OnCreateNewStackRespond,
																		  *_vsObjectFactory.socketFactory,
																		  _settings));
			configTask->configuration->start(_managementStackSourceAddr, message.source);
			return configTask;
		}
	}
    return nullptr;
}

void SouthboundSessionController::processReceivedStorage(StoragePoolPtr storage)
{
	if (!storage)
    {
        Logger::Log(Logger::WARNING, "processReceivedStorage -> invalid storage");
        return;
    }

	auto messageCount = NetworkReceiveMessage::getContainingReceiveMessageCount(*storage);
//	if (messageCount != 1)
//		Logger::Log(Logger::DEBUG, "StorageSize: ", storage->size(), ", MessageCount: ", messageCount);

	for (size_t i = 0; i < messageCount; ++i)
	{
		auto newStorage = _pool->request();
		storage->copyInto(*newStorage);

		auto message = std::make_unique<NetworkReceiveMessage>(_managementStackDestAddr, std::move(newStorage));

		storage->incrementStartIndex(message->size());
		processReceivedMessage(std::move(message));
	}
}

SouthboundSessionController::~SouthboundSessionController()
{
	stop();
//	std::string controllerType = (_nextSessionId % 2 == 0) ? "Init" : "Slave";
//    Logger::Log(Logger::DEBUG, "Destroy ", controllerType, "-Session with state: ", state == ConfigurationState::Ok ? "OK" : std::to_string(static_cast<int>(state)), ", isValid: ", isValid());
}
