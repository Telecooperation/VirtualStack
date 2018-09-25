#include "../../../../VirtualStackSettings.h"
#include "../../../../common/Helper/DomainExtensions.h"
#include "../../../../common/Helper/NetworkExtensions.h"
#include "../../../../interface/ISocketFactory.h"
#include "../Request/NewStackRequest.h"
#include "NewStackRespondConfiguration.h"
#include "NewStackResponse.h"


NewStackRespondConfiguration::NewStackRespondConfiguration(const std::function<void(std::unique_ptr<NewStackResult>)>& onComplete,
														   ISocketFactory& socketFactory,
                                                           const VirtualStackSettings& settings) :
		ExtendedSouthboundConfiguration(NetworkMessageEnum::NewStack),
		_socketFactory(socketFactory),
		_settings(settings),
		_onComplete(onComplete){}

UniqueSocket NewStackRespondConfiguration::run()
{
	//########### Behandeln von CreateStack anfragen von außen
	
	auto tmpRequestMessage = _receiveFromNetwork.pop();
	if(!tmpRequestMessage)
		return nullptr;
	if (!tmpRequestMessage->isValid())
	{
        requestToFinish();
        _state = ConfigurationState::MalformedRequest;
		return nullptr;
	}

	_request = tmpRequestMessage->getElementDeserializedPtr<NewStackRequest>();
	const std::string& sourceIp = _settings.ManagementBindAddress; //So we bind it to the same as the management to support multihoming
	auto stackTransportProtocol = DomainExtensions::getTransportProtocol(_request->stack);
	auto tmpSocket = _socketFactory.createSocketForHolePunching(_request->internetProtocol, stackTransportProtocol, sourceIp);
	if(!tmpSocket)
	{
        requestToFinish();
        _state = ConfigurationState::CreateSocketFailed;
		return nullptr;
	}


    //kopie von managementSockAddr erstellen wegen IP
    //auto requestDestination = destination;
	uint16_t tmpServerSocketPort = tmpSocket->getPort();
	NetworkExtensions::setPort(destination, _request->stackPort);

    NewStackResponse tmpServerPortResponse{tmpServerSocketPort, destination};
    _sendToNetwork.push(&tmpServerPortResponse);

    tmpSocket = _socketFactory.holePunching(std::move(tmpSocket), false, stackTransportProtocol, destination, _settings.IsLocalhost);
	if(!tmpSocket)
	{
        requestToFinish();
        _state = ConfigurationState::HolePunchingFailed;
		return nullptr;
	}

	requestToFinish();
    _state = ConfigurationState::Ok;
	return tmpSocket;
	
	/**
	 * (Prüfen ob nachricht das ist was wir erwarten)
	 * Aus Nachricht den Port der Gegenstelle auslesen
	 * Socket anlegen (Protokoll steht auch in der Nachricht)
	 * Socket an irgendeinen Port hängen
	 * Der Gegenstelle über das Management schicken, welchen Port wir benutzn werden
	 * Holepunchen auf dem Socket
	 */
}

void NewStackRespondConfiguration::onComplete(UniqueSocket &&socket)
{
    auto result = std::make_unique<NewStackResult>(NetworkMessageEnum::NewStack, _state, std::move(socket), destination, std::move(_request));
    _onComplete(std::move(result));
}
