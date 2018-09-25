#include "../IVSFixture.h"
#include "LoopbackKernel.h"
#include <gtest/gtest.h>
#include <stacks/updPlus/addons/UdpFecCreate.h>
#include <DefaultVirtualStackSettings.h>

TEST_F(IVSFixture, UdpPlusFecCreate_Default)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage = pool->request();

    fec.addToStorage(*storage, kernel);

    ASSERT_FALSE(kernel.dataAvailable());
}

TEST_F(IVSFixture, UdpPlusFecCreate_SendImmediately)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 1);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage = pool->request();
    storage->appendDataScalarAfterEnd("Hallo Welt");
    auto storageSize = storage->size();

    fec.addToStorage(*storage, kernel);
    ASSERT_EQ(storage->size() - storageSize, UdpPlusFecHeader::getHeaderSize());

    ASSERT_TRUE(kernel.dataAvailable());

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());


    ASSERT_GE(fecPacket->size(), UdpPlusFecHeader::getHeaderSize());
    fecPacket->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());
    storage->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    ASSERT_EQ(storage->size(), fecPacket->size());
    for (size_t i = 0; i < storage->size(); ++i)
    {
        ASSERT_EQ((*storage)[i], (*fecPacket)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecCreate_LostSmallerPacket)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();
    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");

    fec.addToStorage(*storage1, kernel);
    ASSERT_FALSE(kernel.dataAvailable());

    fec.addToStorage(*storage2, kernel);
    ASSERT_TRUE(kernel.dataAvailable());

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_GE(fecPacket->size(), UdpPlusFecHeader::getHeaderSize());
    auto fecHeader = fecPacket->toTypeAutomatic<UdpPlusFecHeader>();
    ASSERT_EQ(fecHeader.isFec, 1);
    ASSERT_EQ(fecHeader.fecGroup, 1);
    fecPacket->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    auto storage1Header = storage1->toTypeAutomatic<UdpPlusFecHeader>();
    ASSERT_EQ(storage1Header.isFec, 0);
    ASSERT_EQ(storage1Header.fecGroup, 1);

    storage1->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    auto storage2Header = storage2->toTypeAutomatic<UdpPlusFecHeader>();
    ASSERT_EQ(storage2Header.isFec, 0);
    ASSERT_EQ(storage2Header.fecGroup, 1);

    storage2->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    for (size_t i = 0; i < storage2->size(); ++i)
    {
        auto expected = (*storage1)[i] ^ (*fecPacket)[i];
        ASSERT_EQ((*storage2)[i], expected);
    }
}

TEST_F(IVSFixture, UdpPlusFecCreate_ClearFECBuffer)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();
    auto storage3 = pool->request();
    auto storage4 = pool->request();
    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");
    storage3->appendDataScalarAfterEnd("Blah");
    storage4->appendDataScalarAfterEnd("Foo");

    fec.addToStorage(*storage1, kernel);
    ASSERT_FALSE(kernel.dataAvailable());

    fec.addToStorage(*storage2, kernel);
    ASSERT_TRUE(kernel.dataAvailable());

    auto fecPacket = kernel.receivePacket();


    fec.addToStorage(*storage3, kernel);
    ASSERT_FALSE(kernel.dataAvailable());

    fec.addToStorage(*storage4, kernel);
    ASSERT_TRUE(kernel.dataAvailable());
    fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());


    ASSERT_GE(fecPacket->size(), UdpPlusFecHeader::getHeaderSize());

    auto fecHeader = fecPacket->toTypeAutomatic<UdpPlusFecHeader>();
    ASSERT_EQ(fecHeader.isFec, 1);
    ASSERT_EQ(fecHeader.fecGroup, 2);
    fecPacket->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    auto storage3Header = storage3->toTypeAutomatic<UdpPlusFecHeader>();
    ASSERT_EQ(storage3Header.isFec, 0);
    ASSERT_EQ(storage3Header.fecGroup, 2);
    storage3->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    auto storage4Header = storage4->toTypeAutomatic<UdpPlusFecHeader>();
    ASSERT_EQ(storage4Header.isFec, 0);
    ASSERT_EQ(storage4Header.fecGroup, 2);
    storage4->incrementStartIndex(UdpPlusFecHeader::getHeaderSize());

    for (size_t i = 0; i < storage4->size(); ++i)
    {
        auto expected = (*storage3)[i] ^ (*fecPacket)[i];
        ASSERT_EQ((*storage4)[i], expected);
    }
}