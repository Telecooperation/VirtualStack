#pragma once

#include <algorithm>
#include "../../common/Helper/ClassMacros.h"
#include "RemoteControlResultEnum.h"

template <typename T>
struct RemoteControlResult final
{
public:
	RemoteControlResult(T pMessage, RemoteControlResultEnum pResult) :
			message(std::move(pMessage)),
			result(pResult)
	{}
	
	RemoteControlResult(RemoteControlResultEnum pResult) :
			result(pResult)
	{}
	
	T message;
	const RemoteControlResultEnum result;
	
	operator bool() const noexcept
	{
		//message ist ok und es gibt ein future dafür
		return result == RemoteControlResultEnum::Ok && message.valid();
	}
	
	ALLOW_MOVE_SEMANTICS_ONLY(RemoteControlResult);
};