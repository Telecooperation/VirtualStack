#pragma once


#include "../VirtualStackSettings.h"
#include "../common/Allocator/VsObjectFactory.h"
#include "../common/DataStructures/VS/IStack.h"
#include "SouthboundCallbacks.h"
#include "configuration/base/BaseSouthboundConfiguration.h"
#include "model/ConfigurationTask.h"
#include "protocol/NetworkReceiveMessage.h"
#include "protocol/NetworkSendMessage.h"
#include <map>
#include <sys/socket.h>

class SouthboundSessionController final
{
public:
	/**
	 * Create a sessionController for a management Channel between two VirtualStack instances as initiator
	 * @param dest The sockaddr_storage of the destination
	 * @param vsObjectFactory A reference of the vsObjectFactory
	 * @param settings The VirtualStack settings
	 * @param managementFd If initiator its 0, of initiated its > 0
	 */
	SouthboundSessionController(const sockaddr_storage &dest,
								const SouthboundCallbacks& callbacks,
								VsObjectFactory &vsObjectFactory,
								const VirtualStackSettings &settings,
                                UniqueSocket managementFd = nullptr);

	/**
	 * Add a new configuration to this sessionController
	 * @param task The configuration
	 */
	void addConfigurationTask(std::unique_ptr<ConfigurationTask> task);

	/***
	 * Check if containing management has new messages and process it
	 * @return True if successful
	 */
	bool process();

	/***
	 * Stop all pending operations and clear resources
	 */
	void stop();

	/**
	 * Is the managementSocket for this Session still valid or broken and not recoverable
	 * @return True if valid
	 */
	bool isValid() const { return state == ConfigurationState::NotInitialized || state == ConfigurationState::Ok; }

	virtual ~SouthboundSessionController();

	ConfigurationState state;
private:
    /**
     * Extract all unique NetworkReceiveMessages from storage and call processReceiveMessage for each
     * @param storage The buffer containing one to many NetworkReceiveMessages
     */
	inline void processReceivedStorage(StoragePoolPtr storage);

	/**
	 * Add and process a received message from the corresponding management channel for this sessionController
	 * @param message The message to process
	 */
	inline void processReceivedMessage(std::unique_ptr<NetworkReceiveMessage>&& message);
	
	/**
	 * Get the first message to send from this task and send it over the managementConnection
	 * @param task the task to process
	 * @return True if message was sent
	 */
	inline bool processSendMessage(size_t sessionId, ConfigurationTask& task);
	
	/**
	 * Update the nextSessionId in steps of two, so the initiator and the corresponding other configuration wont conflict with sessionIds
	 */
	inline void updateNextSessionId();

	static std::tuple<ConfigurationState, std::unique_ptr<IStack>, sockaddr_storage>
	createManagementStack(const VirtualStackSettings& settings,
						  VsObjectFactory& vsObjectFactory,
						  sockaddr_storage managementStackDestAddr, bool isInitiator);

	inline std::unique_ptr<ConfigurationTask> createTaskFromReceivedMessage(const NetworkReceiveMessage& message);

	const SouthboundCallbacks& _callbacks;
	VsObjectFactory &_vsObjectFactory;
	const VirtualStackSettings& _settings;

	//hier hängt der managementkanal, daher kann hier bestimmt werden wer gerade und wer ungerade ist.
	//man kann den socket abfragen um ein kollektives select zu machen
	//dann gibt es einen tick() unter der annahmen, dass daten vorhanden sind
	//was machen, wenn die managementverbindung wegbricht? versuche es wiederherzustellen?
	
	PoolRef _pool;

	bool _hadOneConfigTask;
	size_t _nextSessionId;
	std::map<size_t, std::unique_ptr<ConfigurationTask>> _sessionIdToConfiguration;

	std::future<std::tuple<ConfigurationState, std::unique_ptr<IStack>, sockaddr_storage>> _createManagementStackPromise;
	/**
	 * The management Stack for this sessionController
	 */
	std::unique_ptr<IStack> _managementStack; //TODO: Multiple stacks? IPv4, IPv6...
	sockaddr_storage _managementStackSourceAddr;
	sockaddr_storage _managementStackDestAddr;
};