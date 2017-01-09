#include "../../../core/db/fields/uuid_map_column/internal/BucketBlockRecord.h"

// todo: remove this namespaces
namespace db {
namespace fields {
namespace uuid_map {



class BucketBlockRecordTests {
public:
    void run() {
        checkInsertInEmptyRecord();
        checkInsert();
        checkInsertDuplicate();

        checkRemoveFromEmptyRecord();
        checkRemove();
        checkReverseRemove();

        checkBinSearchWithZeroElements();
        checkBinSearchWithAbsentElement();
        checkBinSearchWithOneElement();
        checkBinSearchWithPairCountOfElements();
        checkBinSearchWithNonPairCountOfElements();

        checkModifiedAfterInsertion();
        checkModifiedAfterRemoving();
    }

private:
    void checkInsertInEmptyRecord() {
        NodeUUID uuid;
        
        AbstractRecordsHandler::RecordNumber recordNumber(100);

        BucketBlockRecord record(uuid);
        assert(record.mRecordsNumbersCount == 0);

        record.insert(recordNumber);
        assert(record.mRecordsNumbersCount == 1);
        assert(record.mRecordsNumbers[0] == recordNumber);
    }

    void checkInsert() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        for (size_t i = 100; i > 0; --i) {
            record.insert(i);
        }
        assert(record.mRecordsNumbersCount == 100);

        // Check sorting order
        for (size_t i = 0; i < 100; ++i) {
            assert(record.mRecordsNumbers[i] == i + 1);
        }
    }

    void checkInsertDuplicate() {
        NodeUUID uuid;

        BucketBlockRecord record(uuid);
        AbstractRecordsHandler::RecordNumber recordNumber(100);

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

    void checkRemoveFromEmptyRecord() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);
        AbstractRecordsHandler::RecordNumber recordNumber(100);

        assert(!record.remove(recordNumber));
    }

    void checkRemove() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        // populating
        for (size_t i = 0; i < 100; ++i) {
            record.insert(i);
        }

        for (size_t i = 0; i < 100; ++i) {
            assert(record.remove(i));
        }
        assert(record.mRecordsNumbersCount == 0);
    }

    void checkReverseRemove() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        // populating
        for (size_t i = 0; i < 100; ++i) {
            record.insert(i);
        }

        for (size_t i = 100; i > 0; --i) {
            assert(record.remove(i - 1));
        }
        assert(record.mRecordsNumbersCount == 0);
    }

    void checkBinSearchWithZeroElements() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        try {
            record.indexOf(1);
            assert(false); // no throw;

        } catch (IndexError &e) {
        } catch (...) {
            assert(false); // unexpected exception;
        }
    }

    void checkBinSearchWithAbsentElement() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);
        record.insert(1);
        record.insert(2);

        try {
            record.indexOf(100);
            assert(false); // no throw;

        } catch (IndexError &e) {
        } catch (...) {
            assert(false); // unexpected exception;
        }
    }

    void checkBinSearchWithOneElement() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        record.insert(1);
        assert(record.indexOf(1) == 0);
    }

    void checkBinSearchWithPairCountOfElements() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        record.insert(1);
        assert(record.indexOf(1) == 0);

        record.insert(2);
        assert(record.indexOf(2) == 1);
    }

    void checkBinSearchWithNonPairCountOfElements() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);

        record.insert(1);
        assert(record.indexOf(1) == 0);

        record.insert(2);
        assert(record.indexOf(2) == 1);

        record.insert(3);
        assert(record.indexOf(3) == 2);

        record.insert(4);
        assert(record.indexOf(4) == 3);

        record.insert(5);
        assert(record.indexOf(5) == 4);
    }

    void checkModifiedAfterInsertion() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);
        assert(!record.isModified());

        record.insert(1);
        assert(record.isModified());
    }

    void checkModifiedAfterRemoving() {
        NodeUUID uuid;
        BucketBlockRecord record(uuid);
        assert(!record.isModified());

        record.insert(1);
        record.remove(1);
        assert(record.isModified());
    }
};


}
}
}

