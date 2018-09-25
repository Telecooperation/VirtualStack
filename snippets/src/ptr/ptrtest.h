#pragma once


#include <common/DataStructures/Model/Storage.h>

class ptrtest
{
public:
    static void run()
    {

        Pool<Storage> pool{2u, "", [] (Storage& item) { item.reset(); }, 1u};


        StorageUniquePtr uniquePtr{new Storage(1)};
        uniqueMove(std::move(uniquePtr));
        assert(uniquePtr == nullptr);

        uniquePtr.reset(new Storage(1));
        uniqueAssign(std::move(uniquePtr));
        assert(uniquePtr == nullptr);

        StoragePoolPtr poolPtr = pool.request();
        poolMove(std::move(poolPtr));
        assert(uniquePtr == nullptr);

        poolPtr = pool.request();
        poolAssign(std::move(poolPtr));
        assert(uniquePtr == nullptr);
    }


    static void uniqueMove(StorageUniquePtr &&ptr)
    {
    }

    static void uniqueAssign(StorageUniquePtr ptr)
    {

    }

    static void poolMove(StoragePoolPtr &&ptr)
    {

    }

    static void poolAssign(StoragePoolPtr ptr)
    {

    }
};


