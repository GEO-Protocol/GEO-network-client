#include "UUIDMapBlockStorageTest.h"

namespace db {
    namespace uuid_map_block_storage {

        UUIDMapBlockStorageTest::UUIDMapBlockStorageTest() {
            initializeNewPointer();
        }

        UUIDMapBlockStorageTest::~UUIDMapBlockStorageTest() {
            deletePointerToStorage();
        }

        void UUIDMapBlockStorageTest::initializeNewPointer() {
            mMapBlockStorage = new UUIDMapBlockStorage("storage", "test_db.bin");
        }

        void UUIDMapBlockStorageTest::deletePointerToStorage() {
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

            cout << "Every next record longer then previous. Data in record is the same UUID data array that in index block. Next record has concatenated uuids from him index block and all previous index blocks." << endl;

            mMapBlockStorage->write(u1, buffer1, itemSize);
            mMapBlockStorage->write(u2, buffer2, itemSize * 2);
            mMapBlockStorage->write(u3, buffer3, itemSize * 3);

            free(buffer1);
            free(buffer2);
            free(buffer3);
        }

        void UUIDMapBlockStorageTest::readFirstIndex() {
            Block *block = mMapBlockStorage->readFromFile(u1);
            uuids::uuid uuid1;
            memcpy(&uuid1, block->data(), block->bytesCount());
            assert(u1 == uuid1);
            delete block;
        }

        void UUIDMapBlockStorageTest::readSecondIndex() {
            Block *block = mMapBlockStorage->readFromFile(u2);
            uuids::uuid uuid1;
            uuids::uuid uuid2;
            memcpy(&uuid1, block->data(), itemSize);
            memcpy(&uuid2, block->data() + itemSize, itemSize);
            assert(u1 == uuid1);
            assert(u2 == uuid2);
            delete block;
        }

        void UUIDMapBlockStorageTest::readThirdIndex() {
            Block *block = mMapBlockStorage->readFromFile(u3);
            uuids::uuid uuid1;
            uuids::uuid uuid2;
            uuids::uuid uuid3;
            memcpy(&uuid1, block->data(), itemSize);
            memcpy(&uuid2, block->data() + itemSize, itemSize);
            memcpy(&uuid3, block->data() + itemSize * 2, itemSize);
            assert(u1 == uuid1);
            assert(u2 == uuid2);
            assert(u3 == uuid3);
            delete block;
        }

        void UUIDMapBlockStorageTest::readFromFile() {
            deletePointerToStorage();
            initializeNewPointer();
            readFirstIndex();
            readSecondIndex();
            readThirdIndex();
        }

        void UUIDMapBlockStorageTest::start() {
            writeTestCase();
            readFromFile();
        }


    }
}

