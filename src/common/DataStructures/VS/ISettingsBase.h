
#pragma once

#include "../../Helper/ClassMacros.h"
#include "SettingsEntry.h"
#include "SettingsProvider.h"

class ISettingsBase
{
public:
	explicit ISettingsBase(SettingsProvider&& provider) :
			_provider(std::move(provider)),
			_settingsReadFailed(false) {
	}
	virtual ~ISettingsBase();

	bool SettingsReadFailed() const { return _settingsReadFailed; }
	
	SettingsProvider& getProvider() { return _provider; }

	ALLOW_MOVE_SEMANTICS_ONLY(ISettingsBase);
protected:
	SettingsProvider _provider;
	bool _settingsReadFailed;
};


