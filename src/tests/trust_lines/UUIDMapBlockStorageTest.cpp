#include "UUIDMapBlockStorageTest.h"

UUIDMapBlockStorageTest::UUIDMapBlockStorageTest() {}

UUIDMapBlockStorageTest::~UUIDMapBlockStorageTest() {}

void UUIDMapBlockStorageTest::writeTestCase() {
    boost::uuids::uuid u = boost::uuids::random_generator()();
    size_t size = 50;
    byte *array = generateRandomByteArray(size);

    UUIDMapBlockStorage *mapBlockStorage = new UUIDMapBlockStorage("storage.dat");
    mapBlockStorage->write(u, array, size);
    
}

byte *UUIDMapBlockStorageTest::generateRandomByteArray(size_t bytesCount) {
    byte *array = (byte *) malloc(bytesCount);
    for (size_t i=0; i<bytesCount; ++i){
        array[i] = rand();
    }
}

