#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H

#include <iostream>
#include "../../core/db/UUIDMapBlockStorage.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        typedef uint8_t byte;

        class UUIDMapBlockStorageTest{
        public:

            NodeUUID u1;
            NodeUUID u2;
            NodeUUID u3;
            NodeUUID u4;
            NodeUUID u5;

            const size_t itemSize = 16;

            UUIDMapBlockStorage *mMapBlockStorage;

        public:
            UUIDMapBlockStorageTest();

            ~UUIDMapBlockStorageTest();

            void writeTestCase();

            void readFromFile();

            void start();
        };

    }
}

#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
