#pragma once

#include "ConfigurationState.h"
#include "ISerializeable.h"
#include <sys/socket.h>


/**
 *  Extends a received storage with information about the success
 * Is used by a configuration so it can read the content of the storage only if its valid
 */
class IReceiveFromNetwork
{
public:
	explicit IReceiveFromNetwork(const ConfigurationState receiveError);
	IReceiveFromNetwork(const sockaddr_storage& sourceAddr, StoragePoolPtr&& storage);
	
	template<typename TResponse>
	TResponse getElement()
	{
		if(!_data)
			return TResponse();

		return _data->toTypeAutomatic<TResponse>();
	}

	template<typename TResponse>
	TResponse getElementDeserialized()
	{
		static_assert(std::is_base_of<ISerializeable, TResponse>::value, "Type has to implement ISerializable");

		if(!_data)
			return TResponse();

		TResponse response;
		response.deserialize(*_data);
		return response;
	}

	template<typename TResponse>
	std::unique_ptr<TResponse> getElementDeserializedPtr()
	{
		static_assert(std::is_base_of<ISerializeable, TResponse>::value, "Type has to implement ISerializable");

		if(!_data)
			return std::unique_ptr<TResponse>();

		auto response = std::make_unique<TResponse>();
		response->deserialize(*_data);
		return response;
	}
	
	ConfigurationState getError() const;
	bool isValid() const;

	const sockaddr_storage source;
	
	ALLOW_MOVE_SEMANTICS_ONLY(IReceiveFromNetwork);
protected:
	ConfigurationState _receiveError;
	const StoragePoolPtr _data;
};


