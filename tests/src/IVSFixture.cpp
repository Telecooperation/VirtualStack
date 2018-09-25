#include "IVSFixture.h"
#include <DefaultVirtualStackSettings.h>
#include <VirtualStackLoader.h>
#include <virtualStack/device/northboundDevices/DummyNorthboundDevice.h>

IVSFixture::IVSFixture() : Test(), _virtualStackSettings(DefaultVirtualStackSettings::Default2(StackEnum::TCPIPv4, true))
{
    if (_virtualStackSettings.SettingsReadFailed())
        Logger::Log(Logger::ERROR, "Settings for virtualStack failed");
}

IVSFixture::~IVSFixture()
{

}

void IVSFixture::SetUp()
{
    _vsObjectFactory = VirtualStackLoader<DummyNorthboundDevice>::createVSObjectFactory(_virtualStackSettings);
}
