#include "../IVSFixture.h"
#include <gtest/gtest.h>
#include <stacks/updPlus/addons/UdpSequenceNumber.h>
#include <common/DataStructures/Container/RingBufferMove.h>
#include <DefaultVirtualStackSettings.h>

TEST_F(IVSFixture, UdpPlusSequenceNumber_seqNum)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};

    for (size_t i = 1; i < 100; ++i)
    {
        ASSERT_EQ(seqNumModule.getNextSequenceNumber(), i);
    }
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageInOrder)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;

    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        storage->prependDataScalarBeforeStart(i);

        seqNumModule.addStorage(std::move(storage), seqNumModule.getNextSequenceNumber());
        ASSERT_FALSE(storage);

        ASSERT_TRUE(seqNumModule.isAvailable());
    }

    for (size_t j = 0; j < length; ++j)
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }

    ASSERT_FALSE(seqNumModule.isAvailable());
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageReverseOrder)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + length - 1 - i;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
        ASSERT_FALSE(storage);

        if (i == length - 1)
            ASSERT_TRUE(seqNumModule.isAvailable());
        else
            ASSERT_FALSE(seqNumModule.isAvailable());
    }

    for (size_t j = 0; j < length; ++j)
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, firstSeqNum + j);
    }

    ASSERT_FALSE(seqNumModule.isAvailable());
}


TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageMissingNthPacket)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t missing = 5;
    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + length - 1 - i;
        if(nextSeqNum == missing)
            continue;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
        ASSERT_FALSE(storage);

        if (i == length - 1)
            ASSERT_TRUE(seqNumModule.isAvailable());
        else
            ASSERT_FALSE(seqNumModule.isAvailable());
    }

    for (size_t j = firstSeqNum; j < missing; ++j)
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }

    ASSERT_FALSE(seqNumModule.isAvailable());
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageMissing0thPacket)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t missing = 1;
    for (size_t i = 1; i < length; ++i)
    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + length - 1 - i;
        if(nextSeqNum == missing)
            continue;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
        ASSERT_FALSE(storage);

        ASSERT_FALSE(seqNumModule.isAvailable());
    }

    ASSERT_FALSE(seqNumModule.isAvailable());
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageMissingNthPacketAddNthPacket)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t missing = 5;
    for (size_t i = 1; i < length; ++i) //Add packets 9 to 0 except 5
    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + length - 1 - i;
        if(nextSeqNum == missing)
            continue;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
        ASSERT_FALSE(storage);

        if (i == length - 1)
            ASSERT_TRUE(seqNumModule.isAvailable());
        else
            ASSERT_FALSE(seqNumModule.isAvailable());
    }

    for (size_t j = firstSeqNum; j < missing; ++j) //Get packets 0 to 4
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }

    ASSERT_FALSE(seqNumModule.isAvailable()); //Check whether packet 5 is missing

    auto storage = pool->request();

    auto nextSeqNum = missing;
    storage->prependDataScalarBeforeStart(nextSeqNum);

    seqNumModule.addStorage(std::move(storage), nextSeqNum); //Add packet 5
    ASSERT_FALSE(storage);

    for (size_t j = missing; j < length; ++j) //Get packets 5 to 9
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageMultipleTimes)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(128, "");
    const size_t length = 10;
    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();

    for (size_t i = 0; i < length; ++i)
    {
        for (size_t j = 0; j <= i; ++j)
        {
            auto storage = pool->request();
            storage->prependDataScalarBeforeStart(j);

            bool res = seqNumModule.addStorage(std::move(storage), j+firstSeqNum);
            ASSERT_EQ(res, j == i);
            ASSERT_EQ(storage, j < i);

            ASSERT_TRUE(seqNumModule.isAvailable());
        }
    }

    for (size_t j = 0; j < length; ++j)
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }

    ASSERT_FALSE(seqNumModule.isAvailable());
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageOverflow)
{
    //Change sizeof seqNumBuffer to 16
    auto settingsProvider = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsProvider.AddSizeT("SizeOfUdpSequenceNumberBuffer", 16);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsProvider);

    UdpSequenceNumber seqNumModule{*customSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(128, "");

    const auto seqNumBufferSize = customSettings->SizeOfUdpSequenceNumberBuffer;
    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t overflowAdditionalSize = 2;

    for (size_t i = 0; i < seqNumBufferSize; ++i)
    {
        auto storage = pool->request();
        storage->prependDataScalarBeforeStart(i);
        ASSERT_TRUE(seqNumModule.addStorage(std::move(storage), i+firstSeqNum));
        ASSERT_FALSE(storage);

        ASSERT_TRUE(seqNumModule.isAvailable());
    }

    for (size_t i = seqNumBufferSize; i < seqNumBufferSize + overflowAdditionalSize; ++i)
    {
        auto storage = pool->request();
        storage->prependDataScalarBeforeStart(i);
        ASSERT_FALSE(seqNumModule.addStorage(std::move(storage), i+firstSeqNum));
        ASSERT_FALSE(storage);

        ASSERT_TRUE(seqNumModule.isAvailable());
    }

    for (size_t j = 0; j < seqNumBufferSize; ++j)
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }
    ASSERT_FALSE(seqNumModule.isAvailable());
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_addStorageOverflow2Times)
{
    //Change sizeof seqNumBuffer to 16
    auto settingsProvider = DefaultVirtualStackSettings::DefaultFactory(StackEnum::TCPIPv4, true);
    settingsProvider.AddSizeT("SizeOfUdpSequenceNumberBuffer", 16);

    auto customSettings = DefaultVirtualStackSettings::Default(settingsProvider);

    UdpSequenceNumber seqNumModule{*customSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(128, "");

    const auto seqNumBufferSize = customSettings->SizeOfUdpSequenceNumberBuffer;
    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t overflowAdditionalSize = 2;

    for (size_t i = 0; i < seqNumBufferSize; ++i)
    {
        auto storage = pool->request();
        storage->prependDataScalarBeforeStart(i);
        ASSERT_TRUE(seqNumModule.addStorage(std::move(storage), i+firstSeqNum));
        ASSERT_FALSE(storage);

        ASSERT_TRUE(seqNumModule.isAvailable());
    }

    for (size_t i = seqNumBufferSize; i < seqNumBufferSize * 2 + overflowAdditionalSize; ++i)
    {
        auto storage = pool->request();
        storage->prependDataScalarBeforeStart(i);
        ASSERT_FALSE(seqNumModule.addStorage(std::move(storage), i+firstSeqNum));
        ASSERT_FALSE(storage);

        ASSERT_TRUE(seqNumModule.isAvailable());
    }

    for (size_t j = 0; j < seqNumBufferSize; ++j)
    {
        ASSERT_TRUE(seqNumModule.isAvailable());
        auto next = seqNumModule.getNextAvailable();

        ASSERT_TRUE(next);
        ASSERT_GE(next->size(), sizeof(size_t));

        auto seqNum = next->toTypeAutomatic<size_t>();
        ASSERT_EQ(seqNum, j);
    }
    ASSERT_FALSE(seqNumModule.isAvailable());
}


TEST_F(IVSFixture, UdpPlusSequenceNumber_NoneMissingNack)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;


    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();
        storage->prependDataScalarBeforeStart(i);

        seqNumModule.addStorage(std::move(storage), seqNumModule.getNextSequenceNumber());
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);
    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), sizeof(size_t));

    auto largestObserved = storage->toTypeAutomatic<size_t>();
    ASSERT_EQ(length, largestObserved);
}


TEST_F(IVSFixture, UdpPlusSequenceNumber_MissingNthNack)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");
    const size_t length = 10;

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t missing = 5;
    for (size_t i = 0; i < length; ++i)
    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + length - 1 - i;
        if(nextSeqNum == missing)
            continue;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);
    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), 2*sizeof(size_t));

    auto missingSeqNum = storage->toTypeAutomatic<size_t>();
    storage->incrementStartIndex(sizeof(size_t));
    ASSERT_EQ(missing, missingSeqNum);

    auto largestObserved = storage->toTypeAutomatic<size_t>();
    ASSERT_EQ(length-1 + firstSeqNum, largestObserved);
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_MissingAllExceptNthNack)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t notMissing = 10;

    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + notMissing;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);
    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), notMissing*sizeof(size_t) + sizeof(size_t)); //n missing packets + largestObserved

    for (size_t i = 0; i < notMissing; ++i)
    {
        auto missingSeqNum = storage->toTypeAutomatic<size_t>();
        storage->incrementStartIndex(sizeof(size_t));
        ASSERT_EQ(i + firstSeqNum, missingSeqNum);
    }

    auto largestObserved = storage->toTypeAutomatic<size_t>();
    ASSERT_EQ(notMissing + firstSeqNum, largestObserved);
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_MissingAllExceptNthAndMthNack)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t firstNotMissing = 5;
    const size_t secondNotMissing = 13;

    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + firstNotMissing;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
    }

    {
        auto storage = pool->request();

        auto nextSeqNum = firstSeqNum + secondNotMissing;
        storage->prependDataScalarBeforeStart(nextSeqNum);

        seqNumModule.addStorage(std::move(storage), nextSeqNum);
    }

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);
    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), (secondNotMissing-1)*sizeof(size_t) + sizeof(size_t)); //n-1 missing packets + largestObserved

    for (size_t i = 0; i < secondNotMissing; ++i)
    {
        if(i == firstNotMissing)
            continue;
        auto missingSeqNum = storage->toTypeAutomatic<size_t>();
        storage->incrementStartIndex(sizeof(size_t));
        ASSERT_EQ(i + firstSeqNum, missingSeqNum);
    }

    auto largestObserved = storage->toTypeAutomatic<size_t>();
    ASSERT_EQ(secondNotMissing + firstSeqNum, largestObserved);
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_EmptyNack)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");

    auto storage = pool->request();
    seqNumModule.fillWithNAcks(*storage);
    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), 0);
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_NackStorageTooSmallEmpty)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t firstNotMissing = 5;
    const size_t secondNotMissing = 13;

    //first packet
    auto storage = pool->request();
    auto nextSeqNum = firstSeqNum + firstNotMissing;
    storage->prependDataScalarBeforeStart(nextSeqNum);
    seqNumModule.addStorage(std::move(storage), nextSeqNum);

    //second packet
    storage = pool->request();
    nextSeqNum = firstSeqNum + secondNotMissing;
    storage->prependDataScalarBeforeStart(nextSeqNum);
    seqNumModule.addStorage(std::move(storage), nextSeqNum);

    //nack storage
    storage = pool->request();
    storage->incrementStartIndexSafe(storage->freeSpaceForAppend() - sizeof(size_t));

    Logger::setMinLogLevel(Logger::NONE);
    seqNumModule.fillWithNAcks(*storage);
    Logger::setMinLogLevel(Logger::DEBUG);

    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), 0);
}

TEST_F(IVSFixture, UdpPlusSequenceNumber_NackStorageTooSmall)
{
    UdpSequenceNumber seqNumModule{_virtualStackSettings};
    auto pool = _vsObjectFactory->getStorageSendPool(16, "");

    const auto firstSeqNum = seqNumModule.getNextSequenceNumber();
    const size_t firstNotMissing = 5;
    const size_t secondNotMissing = 13;
    const size_t numberOfNacks = 2;

    //first packet
    auto storage = pool->request();
    auto nextSeqNum = firstSeqNum + firstNotMissing;
    storage->prependDataScalarBeforeStart(nextSeqNum);
    seqNumModule.addStorage(std::move(storage), nextSeqNum);

    //second packet
    storage = pool->request();
    nextSeqNum = firstSeqNum + secondNotMissing;
    storage->prependDataScalarBeforeStart(nextSeqNum);
    seqNumModule.addStorage(std::move(storage), nextSeqNum);

    //nack storage
    storage = pool->request();
    storage->incrementStartIndexSafe(storage->freeSpaceForAppend() - (sizeof(size_t) * (numberOfNacks + 1))); //allow to add 2 nacks plus largestObserved

    seqNumModule.fillWithNAcks(*storage);
    ASSERT_TRUE(storage);
    ASSERT_EQ(storage->size(), sizeof(size_t) * (numberOfNacks + 1));

    for (size_t i = 0; i < numberOfNacks; ++i)
    {
        auto val = storage->toTypeAutomatic<size_t>();
        ASSERT_EQ(firstSeqNum + i, val);
        storage->incrementStartIndex(sizeof(size_t));
    }

    ASSERT_EQ(storage->size(), sizeof(size_t));
    auto val = storage->toTypeAutomatic<size_t>();
    ASSERT_EQ(firstSeqNum + numberOfNacks - 1, val);
}