#include "../IVSFixture.h"
#include "LoopbackKernel.h"
#include <DefaultVirtualStackSettings.h>
#include <common/DataStructures/Container/RingBufferMove.h>
#include <gtest/gtest.h>
#include <stacks/updPlus/addons/UdpResending.h>
#include <stacks/updPlus/addons/UdpSequenceNumber.h>

TEST_F(IVSFixture, UdpPlusResending_NoneMissing) //Nack ohne das Pakete fehlen
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);

        seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);

    ASSERT_FALSE(kernel.dataAvailable());
}

TEST_F(IVSFixture, UdpPlusResending_CanBuffer) //Nack ohne das Pakete fehlen
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusResendingBuffer", 4);
    settingsFactory.AddSizeT("SizeOfUdpSequenceNumberBuffer", 4);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);
    UdpSequenceNumber seqNumModule{*customSettings};

    UdpResending resending{*customSettings, *_vsObjectFactory};
    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");

    for (size_t i = 0; i < resending.getCapacity(); ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);

        ASSERT_TRUE(resending.canBuffer());
        ASSERT_TRUE(resending.bufferStorage(storage, seqNum));

        seqNumModule.addStorage(std::move(storage), seqNum);
    }
    ASSERT_FALSE(resending.canBuffer());

    auto storageAfter = pool->request();
    auto seqNum = seqNumModule.getNextSequenceNumber();

    Logger::setMinLogLevel(Logger::NONE);
    ASSERT_FALSE(resending.bufferStorage(storageAfter, seqNum));
    Logger::setMinLogLevel(Logger::DEBUG);
    ASSERT_FALSE(resending.canBuffer());

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);
    ASSERT_TRUE(resending.canBuffer());

    ASSERT_FALSE(kernel.dataAvailable());
}


TEST_F(IVSFixture, UdpPlusResending_AllMissing) // Nack ohne das Pakete empfangen wurden
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(seqNum);
        resending.bufferStorage(storage, seqNum);

        //seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);

    ASSERT_TRUE(kernel.dataAvailable());
    storage = kernel.receivePacket();

    ASSERT_EQ(storage->size(), sizeof(size_t));
    auto resendSeqNum = storage->toTypeAutomatic<size_t>();

    ASSERT_EQ(10, resendSeqNum);
    ASSERT_FALSE(kernel.dataAvailable());
}

TEST_F(IVSFixture, UdpPlusResending_AllBelow2Missing) // Nack mit 0,1 fehlt, 2 ist da -> 0,1 neugesendet werden
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 3;

    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        if(i < length-1)
            continue;
        seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);


    for (size_t i = 0; i < length - 1; ++i)
    {
        ASSERT_TRUE(kernel.dataAvailable());
        auto recvStorage = kernel.receivePacket();

        ASSERT_EQ(recvStorage->size(), sizeof(size_t));
        auto resendSeqNum = recvStorage->toTypeAutomatic<size_t>();

        ASSERT_EQ(i, resendSeqNum);
    }
}

TEST_F(IVSFixture, UdpPlusResending_OddsMissing) // Nack mit 0 ist da, 1 fehlt, 2 ist da,3 fehlt, 4 ist da -> 1,3 neugesendet werden
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 5;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        if(i % 2 == 1)
            continue;
        seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);

    for (size_t i = 0; i < length/2; ++i)
    {
        ASSERT_TRUE(kernel.dataAvailable());
        auto recvStorage = kernel.receivePacket();

        ASSERT_EQ(recvStorage->size(), sizeof(size_t));
        auto resendSeqNum = recvStorage->toTypeAutomatic<size_t>();

        ASSERT_EQ(i*2+1, resendSeqNum);
    }
}

TEST_F(IVSFixture, UdpPlusResending_FirstNotMissingThenMissing) // Nack mit alles bis 2 ist da, danach Nack mit 0 fehlt -> nichts neugesendet
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 2;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto noneMissingStorage = pool->request();
    seqNumModule.fillWithNAcks(*noneMissingStorage);

    resending.processNackInStorage(*noneMissingStorage, kernel);
    ASSERT_FALSE(kernel.dataAvailable());

    UdpSequenceNumber seqNumModule2{_virtualStackSettings};

    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule2.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        if(i == 0)
            seqNumModule2.addStorage(std::move(storage), seqNum);
    }

    auto oneMissingStorage = pool->request();
    seqNumModule2.fillWithNAcks(*oneMissingStorage);

    resending.processNackInStorage(*oneMissingStorage, kernel);
    ASSERT_FALSE(kernel.dataAvailable());
}

TEST_F(IVSFixture, UdpPlusResending_SecondNotMissingThenMissing) // Nack mit alles bis 2 ist da, danach Nack mit 1 fehlt -> nichts neugesendet
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 2;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto noneMissingStorage = pool->request();
    seqNumModule.fillWithNAcks(*noneMissingStorage);

    resending.processNackInStorage(*noneMissingStorage, kernel);
    ASSERT_FALSE(kernel.dataAvailable());

    UdpSequenceNumber seqNumModule2{_virtualStackSettings};

    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule2.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        if(i == 1)
            seqNumModule2.addStorage(std::move(storage), seqNum);
    }

    auto oneMissingStorage = pool->request();
    seqNumModule2.fillWithNAcks(*oneMissingStorage);

    resending.processNackInStorage(*oneMissingStorage, kernel);
    ASSERT_FALSE(kernel.dataAvailable());
}


TEST_F(IVSFixture, UdpPlusResending_NonSentPacketMissing) // Nack mit 3 fehlt, aber nur bis 2 gesendet -> nichts neugesendet
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 4;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        if(i != 2)
        {
            resending.bufferStorage(storage, seqNum);
            seqNumModule.addStorage(std::move(storage), seqNum);
        }
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    Logger::setMinLogLevel(Logger::NONE);
    resending.processNackInStorage(*storage, kernel);
    Logger::setMinLogLevel(Logger::DEBUG);
    ASSERT_FALSE(kernel.dataAvailable());
}

TEST_F(IVSFixture, UdpPlusResending_OddsMissingSplitNack) // Gesplittetes Nack
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 5;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(seqNum);
        resending.bufferStorage(storage, seqNum);
        if(i % 2 == 1)
            continue;
        seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto storage = pool->request();
    storage->incrementStartIndexSafe(storage->freeSpaceForAppend() - sizeof(size_t) * 2);
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);

    ASSERT_TRUE(kernel.dataAvailable());
    storage = kernel.receivePacket();

    ASSERT_EQ(storage->size(), sizeof(size_t));
    auto resendSeqNum = storage->toTypeAutomatic<size_t>();

    ASSERT_EQ(2, resendSeqNum);
    ASSERT_FALSE(kernel.dataAvailable());

    seqNumModule.addStorage(std::move(storage), resendSeqNum); //receive resent packet

    storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);
    resending.processNackInStorage(*storage, kernel);

    ASSERT_TRUE(kernel.dataAvailable());
    storage = kernel.receivePacket();

    ASSERT_EQ(storage->size(), sizeof(size_t));
    resendSeqNum = storage->toTypeAutomatic<size_t>();

    ASSERT_EQ(4, resendSeqNum);
    ASSERT_FALSE(kernel.dataAvailable());
}

TEST_F(IVSFixture, UdpPlusResending_1Missing2Received3Sent)  // Nack mit 1 fehlt, alles bis 2 gesehen, aber 3 schon gesendet -> 1 neugesendet
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    UdpResending resending{_virtualStackSettings, *_vsObjectFactory};
    LoopbackKernel kernel{_virtualStackSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 4;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        if (i % 2 == 0)
            seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);

    resending.processNackInStorage(*storage, kernel);

    ASSERT_TRUE(kernel.dataAvailable());
    storage = kernel.receivePacket();

    ASSERT_EQ(storage->size(), sizeof(size_t));
    auto resendSeqNum = storage->toTypeAutomatic<size_t>();

    ASSERT_EQ(1, resendSeqNum);
    ASSERT_FALSE(kernel.dataAvailable());
}


TEST_F(IVSFixture, UdpPlusResending_ResendingBufferClearTest) //Resending testen, indem SizeOfUdpPlusResendingBuffer = 3, dann 0,1 senden, nack mit 0 erhalten, 2 senden, nack mit 1 fehlt -> buffer zu klein falls nicht gecleart wurde, sonst wird 1 neugesendet
{
    auto settingsFactory = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsFactory.AddSizeT("SizeOfUdpPlusResendingBuffer", 4);
    settingsFactory.AddSizeT("SizeOfUdpSequenceNumberBuffer", 4);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsFactory);


    UdpSequenceNumber seqNumModule{*customSettings};

    UdpResending resending{*customSettings, *_vsObjectFactory};
    LoopbackKernel kernel{*customSettings, *_vsObjectFactory};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 3;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        auto seqNum = seqNumModule.getNextSequenceNumber();
        storage->prependDataScalarBeforeStart(i);
        resending.bufferStorage(storage, seqNum);
        if(i != 1)
            seqNumModule.addStorage(std::move(storage), seqNum);
    }

    auto firstMissingNackStorage = pool->request();
    seqNumModule.fillWithNAcks(*firstMissingNackStorage);

    resending.processNackInStorage(*firstMissingNackStorage, kernel);
    ASSERT_TRUE(kernel.dataAvailable());
    auto storage = kernel.receivePacket();

    ASSERT_EQ(storage->size(), sizeof(size_t));
    auto resendSeqNum = storage->toTypeAutomatic<size_t>();

    ASSERT_EQ(1, resendSeqNum);

    storage = pool->request();
    auto seqNum = seqNumModule.getNextSequenceNumber();
    storage->prependDataScalarBeforeStart(3);
    ASSERT_TRUE(resending.bufferStorage(storage, seqNum));
}