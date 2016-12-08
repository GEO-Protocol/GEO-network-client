#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H

#include <iostream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "../../core/db/UUIDMapBlockStorage.h"

using namespace std;
using namespace db::uuid_map_block_storage;

class UUIDMapBlockStorageTest{
public:

    UUIDMapBlockStorageTest();

    ~UUIDMapBlockStorageTest();

    void writeTestCase();

    void start();
};

#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
