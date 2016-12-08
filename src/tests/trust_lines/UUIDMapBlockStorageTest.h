#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H

#include "UUIDMapBlockStorage.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
namespace storage = db::uuid_map_block_storage;

class UUIDMapBlockStorageTest{
public:
    UUIDMapBlockStorageTest();

    ~UUIDMapBlockStorageTest();

    void writeTestCase();

    byte *generateRandomByteArray(size_t bytesCount);
};

#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
