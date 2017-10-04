#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H

#include <iostream>
#include <assert.h>
#include "../../core/db/uuid_map_block_storage/UUIDMapBlockStorage.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        typedef uint8_t byte;

        class UUIDMapBlockStorageTest{
        public:

            uuids::uuid u1;
            uuids::uuid u2;
            uuids::uuid u3;
            uuids::uuid u4;
            uuids::uuid u5;

            const size_t itemSize = 16;

            UUIDMapBlockStorage *mMapBlockStorage;

        public:
            UUIDMapBlockStorageTest();

            ~UUIDMapBlockStorageTest();

            void initializeNewPointer();

            void deletePointerToStorage();

            void writeTestCase();

            void readFromFile();

            void readFirstIndex();

            void readSecondIndex();

            void readThirdIndex();

            void start();
        };

    }
}

#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGETEST_H
