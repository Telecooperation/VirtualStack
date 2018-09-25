#include "BaseSouthboundConfiguration.h"

BaseSouthboundConfiguration::BaseSouthboundConfiguration(const NetworkMessageEnum pNetworkMessageEnum) :
		networkMessageEnum(pNetworkMessageEnum),
		_sendToNetwork(16),
		_receiveFromNetwork(16),
		_state(ConfigurationState::NotInitialized),
        _linkedPromise(nullptr),
		_onCompleteDone(false),
        _finishState(FinishState::NotInitialized)
{}

BaseSouthboundConfiguration::~BaseSouthboundConfiguration()
{
	stop();
}

ConfigurationState BaseSouthboundConfiguration::getState() const
{
	return _state;
}

bool BaseSouthboundConfiguration::isValid() const
{
	return _state.load() == ConfigurationState::Ok;
}

bool BaseSouthboundConfiguration::isFinished() const
{
	return _onCompleteDone;
}

bool BaseSouthboundConfiguration::hasSendToNetwork() const
{
	return _sendToNetwork.available();
}

const ISerializeable *BaseSouthboundConfiguration::getSendToNetwork()
{
	return _sendToNetwork.pop();
}

void BaseSouthboundConfiguration::addReceiveFromNetwork(std::unique_ptr<IReceiveFromNetwork> message)
{
	_receiveFromNetwork.push(std::move(message));
}

void BaseSouthboundConfiguration::linkPromise(std::promise<ConfigurationState> &promise)
{
    _linkedPromise = &promise;
}

void BaseSouthboundConfiguration::stop() {
	if (isFinished())
		return;

	_state = ConfigurationState::Canceled;
    _finishState = FinishState::Canceled;
	_sendToNetwork.stop();
	_receiveFromNetwork.stop();
}

void BaseSouthboundConfiguration::requestToFinish()
{
    //Try to set RequestsFinish, because requestsFinish can only be set if it is in NotInitialized state
    auto expectedState = FinishState::NotInitialized;
    _finishState.compare_exchange_strong(expectedState, FinishState::RequestsFinish);

	while(_finishState.load() != FinishState::Ok && _finishState.load() != FinishState::Canceled)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool BaseSouthboundConfiguration::requestsFinish() const
{
	return _finishState.load() == FinishState::RequestsFinish;
}

void BaseSouthboundConfiguration::acceptFinish()
{
    _finishState = FinishState::Ok;
}
