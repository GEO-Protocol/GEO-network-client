#include "UUIDMapBlockStorageTest.h"

namespace db {
    namespace uuid_map_block_storage {

        UUIDMapBlockStorageTest::UUIDMapBlockStorageTest() {
            mMapBlockStorage = new UUIDMapBlockStorage("test_db.bin");
        }

        UUIDMapBlockStorageTest::~UUIDMapBlockStorageTest() {
            delete mMapBlockStorage;
        }

        void UUIDMapBlockStorageTest::writeTestCase() {
            byte *buffer1 = (byte *) malloc(itemSize);
            memcpy(buffer1, u1.data, itemSize);

            byte *buffer2 = (byte *)malloc(itemSize * 2);
            memcpy(buffer2, u1.data, itemSize);
            memcpy(buffer2 + itemSize, u2.data, itemSize);

            byte *buffer3 = (byte *)malloc(itemSize * 3);
            memcpy(buffer3, u1.data, itemSize);
            memcpy(buffer3 + itemSize, u2.data, itemSize);
            memcpy(buffer3 + itemSize * 2, u3.data, itemSize);

            cout << "Writing 3 items: " << u1.stringUUID() << " " << u2.stringUUID() << " " << u3.stringUUID() << endl;

            mMapBlockStorage->write(u1, buffer1, itemSize);
            mMapBlockStorage->write(u2, buffer2, itemSize * 2);
            mMapBlockStorage->write(u3, buffer3, itemSize * 3);

            free(buffer1);
            free(buffer2);
            free(buffer3);
        }

        void UUIDMapBlockStorageTest::readFromFile() {
            Block *block = mMapBlockStorage->readFromFile(u3);
            NodeUUID uuid1;
            NodeUUID uuid2;
            NodeUUID uuid3;
            memcpy(&uuid1, block->data(), itemSize);
            memcpy(&uuid2, block->data() + itemSize, itemSize);
            memcpy(&uuid3, block->data() + itemSize * 2, itemSize);
            cout << "Reading data from third index : " << uuid1.stringUUID() << " " << uuid2.stringUUID() << " " << uuid3.stringUUID() << endl;
            delete block;
        }

        void UUIDMapBlockStorageTest::start() {
            writeTestCase();
            readFromFile();
        }

    }
}

