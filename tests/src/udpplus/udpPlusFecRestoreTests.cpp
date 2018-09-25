#include "../IVSFixture.h"
#include "LoopbackKernel.h"
#include <gtest/gtest.h>
#include <stacks/updPlus/addons/UdpFecCreate.h>
#include <DefaultVirtualStackSettings.h>
#include <stacks/updPlus/addons/UdpFecRestore.h>

/**
 * Ein Paket reintun, prüfen, dass kein Paket wiederhergestellt wurde
 * Normale Wiederherstellung (auch 2x parallel (1,3,2,4))
 * Alle Pakete in-order -> keine Wiederherstellung
 * Wiederhergestelltes Paket hat richtige Größe für a) alle haben gleiche größe b) nicht c) alle haben Länge 0 außer eins
 *
 * Überlauf -> Reset des Buffers (auch: danach mit selber Gruppe + BufferSize weiter machen)
 */

TEST_F(IVSFixture, UdpPlusFecRestore_Default)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    UdpFecRestore fec{*customSettings, *_vsObjectFactory};


    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 1;
    UdpPlusFecHeader header{false};
    header.fecGroup = 1;

    storage->prependDataAutomaticBeforeStart(&dataHeader);
    storage->prependDataAutomaticBeforeStart(&header);

    ASSERT_FALSE(fec.receivePacket(*storage));
    ASSERT_FALSE(fec.canRestore());
}

TEST_F(IVSFixture, UdpPlusFecRestore_SendImmediately)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 1);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    UdpFecRestore fec{*customSettings, *_vsObjectFactory};


    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage = pool->request();

    UdpPlusFecHeader header{true};
    header.fecGroup = 1;


    storage->prependDataAutomaticBeforeStart(&header);
    storage->appendDataScalarAfterEnd("Blah");

    ASSERT_TRUE(fec.receivePacket(*storage));
    ASSERT_TRUE(fec.canRestore());

    auto restored = fec.getNextRestored();
    ASSERT_FALSE(fec.canRestore());
    ASSERT_TRUE(restored);

    ASSERT_EQ(storage->size(), restored->size());
    for (size_t i = 0; i < storage->size(); ++i)
    {
        ASSERT_EQ((*storage)[i], (*restored)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecRestore_LostSmallerPacket)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");

    auto storage1 = pool->request();
    auto storage2 = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored);

    ASSERT_EQ(storage1->size(), restored->size());

    //Storage2's fec header was not removed by receivePacket, remove it manually
    storage2->incrementStartIndex(sizeof(UdpPlusFecHeader));

    for (size_t i = 0; i < storage2->size(); ++i)
    {
        ASSERT_EQ((*storage2)[i], (*restored)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecRestore_GetSamePacketTwice)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");

    auto storage1 = pool->request();
    auto storage2 = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto storage1Copy = storage1->copy();

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());

    ASSERT_FALSE(fecRestore.receivePacket(storage1Copy));
    ASSERT_FALSE(fecRestore.canRestore());

    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored);

    ASSERT_EQ(storage1->size(), restored->size());

    //Storage2's fec header was not removed by receivePacket, remove it manually
    storage2->incrementStartIndex(sizeof(UdpPlusFecHeader));

    for (size_t i = 0; i < storage2->size(); ++i)
    {
        ASSERT_EQ((*storage2)[i], (*restored)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecRestore_LostEqualSizedPacket)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt!");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored);

    ASSERT_EQ(storage1->size(), restored->size());

    //Storage2's fec header was not removed by receivePacket, remove it manually
    storage2->incrementStartIndex(sizeof(UdpPlusFecHeader));

    for (size_t i = 0; i < storage2->size(); ++i)
    {
        ASSERT_EQ((*storage2)[i], (*restored)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecRestore_LostPacketOtherWithLengthZero)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    //storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt!");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored);

    //Storage2's fec header was not removed by receivePacket, remove it manually
    storage2->incrementStartIndex(sizeof(UdpPlusFecHeader));
    ASSERT_EQ(storage2->size(), restored->size());


    for (size_t i = 0; i < storage2->size(); ++i)
    {
        ASSERT_EQ((*storage2)[i], (*restored)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecRestore_TwoGroupsParallel)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();
    auto storage3 = pool->request();
    auto storage4 = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 2;
    storage3->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 3;
    storage4->prependDataAutomaticBeforeStart(&dataHeader);

    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");
    storage3->appendDataScalarAfterEnd("Ihr seid schuld");
    storage4->appendDataScalarAfterEnd("an allen Bugs");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    fec.addToStorage(*storage3, kernel);
    fec.addToStorage(*storage4, kernel);

    auto fecPacket2 = kernel.receivePacket();
    fecPacket2->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket2));
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored);

    ASSERT_FALSE(fecRestore.receivePacket(*storage4));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored2 = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored2);

    //Storage2's fec header was not removed by receivePacket, remove it manually
    storage2->incrementStartIndex(sizeof(UdpPlusFecHeader));
    storage3->incrementStartIndex(sizeof(UdpPlusFecHeader));

    ASSERT_EQ(storage1->size(), restored->size());
    ASSERT_EQ(storage3->size(), restored2->size());

    for (size_t i = 0; i < storage2->size(); ++i)
    {
        ASSERT_EQ((*storage2)[i], (*restored)[i]);
    }

    for (size_t i = 0; i < storage3->size(); ++i)
    {
        ASSERT_EQ((*storage3)[i], (*restored2)[i]);
    }
}

TEST_F(IVSFixture, UdpPlusFecRestore_NoLostPackets)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();

    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_FALSE(fecRestore.receivePacket(*storage2));
    ASSERT_FALSE(fecRestore.canRestore());

    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_FALSE(fecRestore.canRestore());
}


TEST_F(IVSFixture, UdpPlusFecRestore_Overflow)
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecGroupSize", 2);
    settingsFactory.AddSizeT("SizeOfUdpPlusFecRestoreBuffer", 2);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);

    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    UdpFecCreate fec{*customSettings, *_vsObjectFactory};
    UdpFecRestore fecRestore{*customSettings, *_vsObjectFactory};

    auto pool = _vsObjectFactory->getStorageSendPool(16, "sendPool");
    auto storage1 = pool->request();
    auto storage2 = pool->request();
    auto storage3 = pool->request();
    auto storage4 = pool->request();


    UdpPlusDataHeader dataHeader;
    dataHeader.seqNum = 0;
    storage1->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 1;
    storage2->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 2;
    storage3->prependDataAutomaticBeforeStart(&dataHeader);

    dataHeader.seqNum = 3;
    storage4->prependDataAutomaticBeforeStart(&dataHeader);

    storage1->appendDataScalarAfterEnd("Hallo");
    storage2->appendDataScalarAfterEnd("Welt");
    storage3->appendDataScalarAfterEnd("Ihr seid schuld");
    storage4->appendDataScalarAfterEnd("an allen Bugs");

    fec.addToStorage(*storage1, kernel);
    fec.addToStorage(*storage2, kernel);

    auto fecPacket = kernel.receivePacket();
    fecPacket->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    //Generate overflow
    fec.addToStorage(*storage2, kernel);
    fec.addToStorage(*storage2, kernel);

    kernel.receivePacket();

    fec.addToStorage(*storage3, kernel);
    fec.addToStorage(*storage4, kernel);

    auto fecPacket2 = kernel.receivePacket();
    fecPacket2->incrementStartIndex(UdpPlusHeader::getHeaderSize());

    ASSERT_FALSE(fecRestore.receivePacket(*storage1));
    ASSERT_FALSE(fecRestore.canRestore());

    ASSERT_FALSE(fecRestore.receivePacket(*storage3));
    ASSERT_FALSE(fecRestore.canRestore());

    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket));
    ASSERT_FALSE(fecRestore.canRestore());

    ASSERT_TRUE(fecRestore.receivePacket(*fecPacket2));
    ASSERT_TRUE(fecRestore.canRestore());

    auto restored2 = fecRestore.getNextRestored();
    ASSERT_FALSE(fecRestore.canRestore());
    ASSERT_TRUE(restored2);




    //Storage2's fec header was not removed by receivePacket, remove it manually
    storage2->incrementStartIndex(sizeof(UdpPlusFecHeader));
    storage4->incrementStartIndex(sizeof(UdpPlusFecHeader));

    for (size_t i = 0; i < storage4->size(); ++i)
    {
        ASSERT_EQ((*storage4)[i], (*restored2)[i]);
    }
}