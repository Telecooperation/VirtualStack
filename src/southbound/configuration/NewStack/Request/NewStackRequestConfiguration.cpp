#include "../../../../VirtualStackSettings.h"
#include "../../../../common/Helper/DomainExtensions.h"
#include "../../../../common/Helper/NetworkExtensions.h"
#include "../../../../interface/ISocketFactory.h"
#include "../../../protocol/NetworkMessageEnum.h"
#include "../Respond/NewStackResponse.h"
#include "NewStackRequestConfiguration.h"


NewStackRequestConfiguration::NewStackRequestConfiguration(std::unique_ptr<NewStackRequest> request,
														   const std::function<void(std::unique_ptr<NewStackResult>)>& onComplete,
														   ISocketFactory& socketFactory,
														   const VirtualStackSettings& settings) :
		ExtendedSouthboundConfiguration(NetworkMessageEnum::NewStack),
		_request(std::move(request)),
		_socketFactory(socketFactory),
        _sourceByPartner(),
		_settings(settings),
		_onComplete(onComplete)
{}

UniqueSocket NewStackRequestConfiguration::run()
{
	//########### Verbindung per HolePunching aushandeln

	const std::string& sourceIp = _settings.ManagementBindAddress; //So we bind it to the same as the management to support multihoming
    auto stackTransportProtocol = DomainExtensions::getTransportProtocol(_request->stack);
	auto tmpSocket = _socketFactory.createSocketForHolePunching(_request->internetProtocol,
                                                                        stackTransportProtocol,
                                                                        sourceIp);
	
	if(!tmpSocket)
	{
		requestToFinish();
        _state = ConfigurationState::HolePunchingGetFreePortFailed;
		return nullptr;
	}

	_request->stackPort = tmpSocket->getPort(); //hole vom system generierten port aus socket raus. steht nicht in sockaddr_Storage
	_sendToNetwork.push(_request.get());
	auto tmpAnswer = _receiveFromNetwork.pop();
	if (!tmpAnswer)
    {
		requestToFinish();
        _state = ConfigurationState::Canceled;
        return nullptr;
    }

	if(!tmpAnswer->isValid())
	{
		requestToFinish();
        _state = ConfigurationState::RemoteGetFreePortFailed;
		return nullptr;
	}
	
	auto newStackResponse = tmpAnswer->getElementDeserializedPtr<NewStackResponse>();
	_sourceByPartner = newStackResponse->source;
	//auto tmpServerSocketAddr = destination;
	NetworkExtensions::setPort(destination, newStackResponse->remotePort);
	tmpSocket = _socketFactory.holePunching(std::move(tmpSocket), true, stackTransportProtocol, destination, _settings.IsLocalhost);
	if(!tmpSocket)
	{
		requestToFinish();
        _state = ConfigurationState::HolePunchingFailed;
		return nullptr;
	}
//
//	//########### Ergebnis in promise schreiben

    requestToFinish();
    _state = ConfigurationState::Ok;
	return tmpSocket;
//	connectionRequest.socketPromise.set_value(CreateSocketResult(std::move(tmpSocket), SouthboundResultEnum::Ok));
	
	/**
	 * Lege Socket an (+ bind 0)
	 * Lese Port des Sockets aus
	 * Schreibe Port in Nachricht
	 * Sende Nachricht zum Server -> Server sendet Antwort und startet mit Holepunching
	 * Warte auf Antwort auf dem Managementkanal
	 * Lese Serverport aus
	 * Starte Holepunching auf den Serverport
	 * -> Verbindung aufgebaut
	 */
}

void NewStackRequestConfiguration::onComplete(UniqueSocket &&socket)
{
	auto result = std::make_unique<NewStackResult>(NetworkMessageEnum::NewStack, _state, std::move(socket), destination, std::move(_request));
	result->sourceByPartner = _sourceByPartner;
	_onComplete(std::move(result));
}
