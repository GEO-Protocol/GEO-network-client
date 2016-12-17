#include "../../../core/db/fields/uuid_map_column/internal/BucketBlock.h"

namespace db {
namespace fields {
namespace uuid_map {


class BucketBlockTests {
public:
    void run() {
        checkInsertInEmptyBlock();
        checkInsertInSameUUID();
//        checkInsertInDifferentUUID();
//        checkInsertDuplicate();
//        checkRemoveFromEmptyRecord();
//        checkRemoveWithSameUUID();
//        checkInsertInDifferentUUID();
//        checkBinSearchWithZeroElements();
//        checkBinSearchWithAbsentElement();
//        checkInsertOrder();
//        checkBinSearchWithPairCountOfElements();
//        checkBinSearchWithNonPairCountOfElements();
//        checkModifiedAfterInsertion();
//        checkModifiedAfterRemoving();
    }

private:
    void checkInsertInEmptyBlock() {
        NodeUUID uuid;
        AbstractRecordsHandler::RecordNumber recordNumber(1);

        BucketBlock block;
        assert(block.mRecordsCount == 0);

        block.insert(uuid, recordNumber);
        assert(block.mRecordsCount == 1);
        assert(NodeUUID::compare(block.mRecords[0].uuid(), uuid) == NodeUUID::EQUAL);
        assert(block.mRecords[0].mRecordsNumbersCount == 1);
    }

    void checkInsertInSameUUID() {
        NodeUUID uuid;
        BucketBlock block;
        assert(block.mRecordsCount == 0);

        for (size_t i = 100; i > 0; --i) {
            block.insert(uuid, i);
        }
        assert(block.mRecordsCount == 1);
        assert(block.mRecords[0].mRecordsNumbersCount == 100);
    }

    void checkInsertInDifferentUUID() {
        BucketBlock block;
        assert(block.mRecordsCount == 0);

        for (size_t i = 100; i > 0; --i) {
            block.insert(NodeUUID(), i);
        }
        assert(block.mRecordsCount == 100);
        for (size_t i = 0; i < 100; ++i) {
            assert(block.mRecords[i].mRecordsNumbersCount == 1);
        }
    }

    void checkInsertOrder() {
        BucketBlock block;
        assert(block.mRecordsCount == 0);

        NodeUUID uuid1;
        NodeUUID uuid2;
        NodeUUID uuid3;

        // zero the uuids for order predictability reasons
        for (size_t i = 0; i < 16; ++i) {
            uuid1.data[i] = 0;
            uuid2.data[i] = 0;
            uuid3.data[i] = 0;
        }

        uuid1.data[15] = 1;
        uuid2.data[15] = 2;
        uuid3.data[15] = 3;

        block.insert(uuid3, 1);
        block.insert(uuid2, 1);
        block.insert(uuid1, 1);
        assert(block.mRecordsCount == 3);
        assert(block.mRecords[0].uuid().data[15] == 1);
        assert(block.mRecords[1].uuid().data[15] == 2);
        assert(block.mRecords[2].uuid().data[15] == 3);
    }

    void checkInsertDuplicate() {
        NodeUUID uuid;
        BucketBlock block;
        assert(block.mRecordsCount == 0);

        try {
            block.insert(uuid, 1);
            block.insert(uuid, 1);
            assert(false); // nothrow

        } catch (ConflictError &e) {}
        catch (...) {
            assert(false); // unexpected exception
        }
    }

    void checkRemoveFromEmptyRecord() {
        NodeUUID uuid;
        BucketBlock block;

        assert(!block.remove(uuid, 1));
    }

    void checkRemoveWithSameUUID() {
        NodeUUID uuid;
        BucketBlock block;

        // populating
        for (size_t i = 0; i < 100; ++i) {
            block.insert(uuid, i);
        }

        for (size_t i = 0; i < 100; ++i) {
            assert(block.remove(uuid, i));
        }
        assert(block.mRecordsCount == 0);
    }

    void checkRemoveWithDifferentUUID() {
        BucketBlock block;

        std::vector<NodeUUID> uuids;
        for (size_t i = 0; i < 100; ++i) {
            uuids.push_back(NodeUUID());
        }

        // populating
        for (size_t i = 0; i < 100; ++i) {
            block.insert(uuids.at(i), i);
        }

        for (size_t i = 0; i < 100; ++i) {
            assert(block.remove(uuids.at(i), i));
        }
        assert(block.mRecordsCount == 0);
    }

    void checkBinSearchWithZeroElements() {
        NodeUUID uuid;
        BucketBlock block;

        try {
            block.recordIndexByUUID(NodeUUID());
            assert(false); // no throw;

        } catch (IndexError &e) {
        } catch (...) {
            assert(false); // unexpected exception;
        }
    }

    void checkBinSearchWithAbsentElement() {
        NodeUUID uuid1;
        NodeUUID uuid2;
        BucketBlock block;
        block.insert(uuid1, 1);
        block.insert(uuid2, 2);

        try {
            block.recordIndexByUUID(NodeUUID());
            assert(false); // no throw;

        } catch (IndexError &e) {
        } catch (...) {
            assert(false); // unexpected exception;
        }
    }

    void checkBinSearchWithOneElement() {
        NodeUUID uuid;
        BucketBlock block;

        block.insert(uuid, 1);
        assert(block.recordIndexByUUID(uuid) == 0);
    }

    void checkBinSearchWithPairCountOfElements() {
        NodeUUID uuid1;
        NodeUUID uuid2;
        BucketBlock block;

        block.insert(uuid1, 1);
        assert(block.recordIndexByUUID(uuid1) == 0);

        block.insert(uuid2, 2);
        assert(block.recordIndexByUUID(uuid2) == 1);
    }

    void checkBinSearchWithNonPairCountOfElements() {
        BucketBlock block;

        NodeUUID uuid1;
        NodeUUID uuid2;
        NodeUUID uuid3;
        NodeUUID uuid4;
        NodeUUID uuid5;

        for (size_t i = 0; i < 16; i++) {
            uuid1.data[i] = 0;
            uuid2.data[i] = 0;
            uuid3.data[i] = 0;
            uuid4.data[i] = 0;
            uuid5.data[i] = 0;
        }

        uuid1.data[15] = 1;
        uuid2.data[15] = 2;
        uuid3.data[15] = 3;
        uuid4.data[15] = 4;
        uuid5.data[15] = 5;

        block.insert(uuid5, 5);
        assert(block.recordIndexByUUID(uuid5) == 0);

        block.insert(uuid4, 4);
        assert(block.recordIndexByUUID(uuid4) == 0);

        block.insert(uuid3, 3);
        assert(block.recordIndexByUUID(uuid3) == 0);

        block.insert(uuid2, 2);
        assert(block.recordIndexByUUID(uuid2) == 0);

        block.insert(uuid1, 1);
        assert(block.recordIndexByUUID(uuid1) == 0);
    }

    void checkModifiedAfterInsertion() {
        NodeUUID uuid;
        BucketBlock block;
        assert(!block.isModified());

        block.insert(NodeUUID(), 1);
        assert(block.isModified());
    }

    void checkModifiedAfterRemoving() {
        BucketBlock block;
        assert(!block.isModified());

        NodeUUID uuid;
        block.insert(uuid, 1);
        block.remove(uuid, 1);
        assert(block.isModified());
    }
};

}
}
}
