#include "../IVSFixture.h"
#include <gtest/gtest.h>
#include <stacks/updPlus/addons/UdpFlowWindowSend.h>
#include <stacks/updPlus/addons/UdpFlowWindowRecv.h>

/*
 * sendAllowed wenn quota = 0 und != 0
 * keine pakete empfangen -> update: 0
 * alle pakete empfangen -> update: n
 */


TEST_F(IVSFixture, UdpPlusFlowWindow_TestSendAllowed)
{
    UdpFlowWindowSend flowWindow{_virtualStackSettings, 1};

    ASSERT_TRUE(flowWindow.sendDataAllowed());
    flowWindow.sentOnePacket();
    ASSERT_FALSE(flowWindow.sendDataAllowed());
}

TEST_F(IVSFixture, UdpPlusFlowWindow_NoPacketsReceived)
{
    UdpFlowWindowRecv flowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();
    flowWindow.storeWindowUpdate(*storage, 8);

    ASSERT_EQ(storage->size(), sizeof(size_t) * 2);

    auto maxFreeSlots = storage->toTypeAutomatic<size_t>();
    auto windowUpdate = storage->toTypeAutomatic<size_t>(sizeof(size_t));

    ASSERT_EQ(maxFreeSlots, 8);
    ASSERT_EQ(windowUpdate, 0);
}

TEST_F(IVSFixture, UdpPlusFlowWindow_AllPacketsReceivedAndReleased)
{
    UdpFlowWindowRecv flowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();

    for(int i = 0; i < 8; ++i)
    {
        flowWindow.receiveOnePacket();
    }

    flowWindow.storeWindowUpdate(*storage, 8);

    ASSERT_EQ(storage->size(), sizeof(size_t) * 2);

    auto maxFreeSlots = storage->toTypeAutomatic<size_t>();
    auto windowUpdate = storage->toTypeAutomatic<size_t>(sizeof(size_t));

    ASSERT_EQ(maxFreeSlots, 8);
    ASSERT_EQ(windowUpdate, 8);
}

TEST_F(IVSFixture, UdpPlusFlowWindow_AllPacketsReceivedZeroReleased)
{
    UdpFlowWindowRecv flowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();

    for(int i = 0; i < 8; ++i)
    {
        flowWindow.receiveOnePacket();
    }

    flowWindow.storeWindowUpdate(*storage, 0);

    ASSERT_EQ(storage->size(), sizeof(size_t) * 2);

    auto maxFreeSlots = storage->toTypeAutomatic<size_t>();
    auto windowUpdate = storage->toTypeAutomatic<size_t>(sizeof(size_t));

    ASSERT_EQ(maxFreeSlots, 8);
    ASSERT_EQ(windowUpdate, 0);
}

TEST_F(IVSFixture, UdpPlusFlowWindow_AllPacketsReceivedThreeReleased)
{
    UdpFlowWindowRecv flowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();

    for(int i = 0; i < 8; ++i)
    {
        flowWindow.receiveOnePacket();
    }

    flowWindow.storeWindowUpdate(*storage, 3);

    ASSERT_EQ(storage->size(), sizeof(size_t) * 2);

    auto maxFreeSlots = storage->toTypeAutomatic<size_t>();
    auto windowUpdate = storage->toTypeAutomatic<size_t>(sizeof(size_t));

    ASSERT_EQ(maxFreeSlots, 8);
    ASSERT_EQ(windowUpdate, 3);
}

TEST_F(IVSFixture, UdpPlusFlowWindow_AllPacketsReceivedThreeReleasedTwoReceivedThreeReleased)
{
    UdpFlowWindowRecv flowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();

    for(int i = 0; i < 8; ++i)
    {
        flowWindow.receiveOnePacket();
    }

    flowWindow.storeWindowUpdate(*storage, 3);
    storage->reset();

    flowWindow.receiveOnePacket();
    flowWindow.receiveOnePacket();


    flowWindow.storeWindowUpdate(*storage, 4);

    ASSERT_EQ(storage->size(), sizeof(size_t) * 2);

    auto maxFreeSlots = storage->toTypeAutomatic<size_t>();
    auto windowUpdate = storage->toTypeAutomatic<size_t>(sizeof(size_t));

    ASSERT_EQ(maxFreeSlots, 8);
    ASSERT_EQ(windowUpdate, 6);

    /*
     * Empfange 8
     * 3 im Buffer frei -> 3 freigegeben
     * -> 5 im Buffer
     * -> erstelle Update
     * Empfange 2
     * 4 im Buffer frei -> 3 freigegeben
     */
}

TEST_F(IVSFixture, UdpPlusFlowWindow_TestUpdateSendQuota)
{
    UdpFlowWindowSend sendFlowWindow{_virtualStackSettings, 8};
    UdpFlowWindowRecv recvFlowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();

    for(int i = 0; i < 8; ++i)
    {
        sendFlowWindow.sentOnePacket();
        recvFlowWindow.receiveOnePacket();
    }

    recvFlowWindow.storeWindowUpdate(*storage, 3);
    ASSERT_FALSE(sendFlowWindow.sendDataAllowed());
    sendFlowWindow.updateSendQuota(*storage);

    for(int i = 0; i < 3; ++i)
    {
        ASSERT_TRUE(sendFlowWindow.sendDataAllowed());
        sendFlowWindow.sentOnePacket();
    }
    ASSERT_FALSE(sendFlowWindow.sendDataAllowed());

    recvFlowWindow.receiveOnePacket();
    recvFlowWindow.receiveOnePacket();

    storage->reset();
    recvFlowWindow.storeWindowUpdate(*storage, 4);

    ASSERT_FALSE(sendFlowWindow.sendDataAllowed());
    sendFlowWindow.updateSendQuota(*storage);

    for(int i = 0; i < 3; ++i)
    {
        ASSERT_TRUE(sendFlowWindow.sendDataAllowed());
        sendFlowWindow.sentOnePacket();
    }
    ASSERT_FALSE(sendFlowWindow.sendDataAllowed());
}

TEST_F(IVSFixture, UdpPlusFlowWindow_TestUpdateSendQuotaWrongWindowUpdateOrder)
{
    UdpFlowWindowSend sendFlowWindow{_virtualStackSettings, 8};
    UdpFlowWindowRecv recvFlowWindow{_virtualStackSettings, 8, 8};

    auto pool = _vsObjectFactory->getStorageSendPool(8, "TestPool");
    auto storage = pool->request();

    for(int i = 0; i < 8; ++i)
    {
        sendFlowWindow.sentOnePacket();
        recvFlowWindow.receiveOnePacket();
    }

    recvFlowWindow.storeWindowUpdate(*storage, 3);
    ASSERT_FALSE(sendFlowWindow.sendDataAllowed());

    recvFlowWindow.receiveOnePacket();
    recvFlowWindow.receiveOnePacket();

    auto storage2 = pool->request();
    recvFlowWindow.storeWindowUpdate(*storage2, 4);


    sendFlowWindow.updateSendQuota(*storage2);
    sendFlowWindow.updateSendQuota(*storage);

    for(int i = 0; i < 6; ++i)
    {
        ASSERT_TRUE(sendFlowWindow.sendDataAllowed());
        sendFlowWindow.sentOnePacket();
    }
    ASSERT_FALSE(sendFlowWindow.sendDataAllowed());
}