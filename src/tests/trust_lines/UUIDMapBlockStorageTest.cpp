#include "UUIDMapBlockStorageTest.h"

UUIDMapBlockStorageTest::UUIDMapBlockStorageTest() {}

UUIDMapBlockStorageTest::~UUIDMapBlockStorageTest() {}

void UUIDMapBlockStorageTest::writeTestCase() {
    NodeUUID u;

    UUIDMapBlockStorage *mapBlockStorage = new UUIDMapBlockStorage(string("storage.bin"));
    try{
        mapBlockStorage->write(u, u.data, 16);
        mapBlockStorage->write(u, u.data, 16);
    } catch(std::exception &e){
        cout << e.what() << endl;
    }


    delete mapBlockStorage;
}

void UUIDMapBlockStorageTest::start() {
    writeTestCase();
}