#include "IReceiveFromNetwork.h"

IReceiveFromNetwork::IReceiveFromNetwork(const ConfigurationState receiveError) :
		source(),
		_receiveError(receiveError),
		_data()
{}


IReceiveFromNetwork::IReceiveFromNetwork(const sockaddr_storage& sourceAddr, StoragePoolPtr&& storage) :
		source(sourceAddr),
		_receiveError(ConfigurationState::Ok),
	 	_data(std::move(storage))
{
	
}

ConfigurationState IReceiveFromNetwork::getError() const
{
	return _receiveError;
}

bool IReceiveFromNetwork::isValid() const
{
	return _receiveError == ConfigurationState::Ok;
}

