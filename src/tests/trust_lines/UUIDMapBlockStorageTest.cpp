#include "UUIDMapBlockStorageTest.h"

UUIDMapBlockStorageTest::UUIDMapBlockStorageTest() {}

UUIDMapBlockStorageTest::~UUIDMapBlockStorageTest() {}

void UUIDMapBlockStorageTest::writeTestCase() {
    NodeUUID u;
    NodeUUID u1;

    UUIDMapBlockStorage *mapBlockStorage = new UUIDMapBlockStorage(string("storage.bin"));
    try{
        mapBlockStorage->write(u, u.data, 16);
        mapBlockStorage->write(u1, u1.data, 16);
        mapBlockStorage->erase(u);
        mapBlockStorage->vacuum();
    } catch(std::exception &e){
        cout << e.what() << endl;
    }


    delete mapBlockStorage;
}

void UUIDMapBlockStorageTest::start() {
    writeTestCase();
}