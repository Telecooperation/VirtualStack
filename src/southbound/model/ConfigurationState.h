#pragma once

#include <cstdint>
#include <string>

enum class ConfigurationState : uint8_t
{
	NotInitialized,
	Running,
	Canceled,
	Timeout,
	Ok,
    ManagementClosed,
    NoConfigurationsClosed,
	ManagementListenerPortInUse,
	ManagementListenerSocketCreationFailed,
	ManagementListenerSocketReuseFailed,
	ManagementListenerInterfaceBindFailed,
	ManagementListenerSetAsListenFailed,
	ManagementConnectionCreateFailed,
	SessionHolePunchingFailed,
	SessionSocketBindFailed,
	HolePunchingFailed,
	HolePunchingGetFreePortFailed,
	RemoteGetFreePortFailed,
	CreateSocketFailed,
	MalformedRequest,
	BindingToNetworkInterfaceByNameFailed,
	EPollAddFailed,
	SettingsForStackInvalid
};

class ConfigurationStateWrapper
{
public:
    static std::string toString(ConfigurationState state)
    {
        switch (state)
        {
            case ConfigurationState::NotInitialized:
                return "NotInitialized";
            case ConfigurationState::Running:
                return "Running";
            case ConfigurationState::Canceled:
                return "Canceled";
            case ConfigurationState::Timeout:
                return "Timeout";
            case ConfigurationState::Ok:
                return "Ok";
            case ConfigurationState::ManagementClosed:
                return "ManagementClosed";
            case ConfigurationState::NoConfigurationsClosed:
                return "NoConfigurationsClosed";
            case ConfigurationState::ManagementListenerPortInUse:
                return "ManagementListenerPortInUse";
            case ConfigurationState::ManagementListenerSocketCreationFailed:
                return "ManagementListenerSocketCreationFailed";
            case ConfigurationState::ManagementListenerSocketReuseFailed:
                return "ManagementListenerSocketReuseFailed";
            case ConfigurationState::ManagementListenerInterfaceBindFailed:
                return "ManagementListenerInterfaceBindFailed";
            case ConfigurationState::ManagementListenerSetAsListenFailed:
                return "ManagementListenerSetAsListenFailed";
            case ConfigurationState::ManagementConnectionCreateFailed:
                return "ManagementConnectionCreateFailed";
            case ConfigurationState::SessionHolePunchingFailed:
                return "SessionHolePunchingFailed";
            case ConfigurationState::SessionSocketBindFailed:
                return "SessionSocketBindFailed";
            case ConfigurationState::HolePunchingFailed:
                return "HolePunchingFailed";
            case ConfigurationState::HolePunchingGetFreePortFailed:
                return "HolePunchingGetFreePortFailed";
            case ConfigurationState::RemoteGetFreePortFailed:
                return "RemoteGetFreePortFailed";
            case ConfigurationState::CreateSocketFailed:
                return "CreateSocketFailed";
            case ConfigurationState::MalformedRequest:
                return "MalformedRequest";
            case ConfigurationState::BindingToNetworkInterfaceByNameFailed:
                return "BindingToNetworkInterfaceByNameFailed";
            case ConfigurationState::EPollAddFailed:
                return "EPollAddFailed";
            case ConfigurationState::SettingsForStackInvalid:
                return "SettingsForStackInvalid";
        }
        return "Missing ConfigurationState toString()";
    }
};

