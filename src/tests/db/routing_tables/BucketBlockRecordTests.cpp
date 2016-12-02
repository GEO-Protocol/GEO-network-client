#include "../../../core/db/routing_tables/internal/BucketBlockRecord.h"

#include <assert.h>


namespace db {
namespace routing_tables {

class BucketBlockRecordTests {
public:
    void run(){
        checkInsertInEmptyRecord();
        checkInsert();
        checkInsertDuplicate();

        checkRemoveFromEmptyRecord();
        checkRemove();
        checkReverseRemove();
    }

private:
    void checkInsertInEmptyRecord() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);
        RecordNumber recordNumber(100);

        record.insert(recordNumber);

        assert(record.mRecordsNumbersCount == 1);
        assert(record.mRecordsNumbers[0] == recordNumber);
    }

    void checkInsert() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        for (size_t i=100; i>0; --i){
            record.insert(i);
        }
        assert(record.mRecordsNumbersCount == 100);

        // Check sorting order
        for (size_t i=0; i<100; ++i){
            assert(record.mRecordsNumbers[i] == i+1);
        }
    }

    void checkInsertDuplicate() {
        NodeUUID uuid;

        BucketBlockRecord record(uuid);
        RecordNumber recordNumber(100);

        try {
            record.insert(recordNumber);
            record.insert(recordNumber);
            record.insert(recordNumber);
            assert(true); // no throw

        } catch (ConflictError &e) {

        } catch (...) {
            assert(true); // not expected exception
        }

        assert(record.mRecordsNumbersCount == 1);
        assert(record.mRecordsNumbers[0] == recordNumber);
    }

    void checkRemoveFromEmptyRecord(){
        NodeUUID uuid;
        BucketBlockRecord record(uuid);
        RecordNumber recordNumber(100);

        assert(!record.remove(recordNumber));
    }

    void checkRemove() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        // populating
        for (size_t i=0; i<100; ++i){
            record.insert(i);
        }

        for (size_t i=0; i<100; ++i){
            assert(record.remove(i));
        }
        assert(record.mRecordsNumbersCount == 0);
    }

    void checkReverseRemove() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        // populating
        for (size_t i=0; i<100; ++i){
            record.insert(i);
        }

        for (size_t i=100; i>0; --i){
            assert(record.remove(i - 1));
        }
        assert(record.mRecordsNumbersCount == 0);
    }

};

}
}

