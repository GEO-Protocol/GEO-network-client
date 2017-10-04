#include "../../../../core/db/fields/uuid_column/UUIDColumn.h"


using namespace db;
using namespace db::fields;


namespace f = boost::filesystem;


class UUIDColumnTests {
public:
    void run() {
        checkInternalFilesCreation();
        checkAtomicSet();
        checkNonAtomicSet();
        checkSetSeveralRecordNumbersIntoSameRecord();

        checkAtomicRemoving();
        checkAtomicRemovingTC2();

//        performanceOfWriting();

        clean();
    };

protected:
    // Testable class, that allows acces to the private methods and fields.
    class TUUIDColumn:
        public UUIDColumn {
        friend class UUIDColumnTests;

    public:
        using UUIDColumn::UUIDColumn;
    };


    // Returns path to the directory,
    // into which all the tests must be done.
    const f::path path() const {
        return f::path("tests/db/uuid_column/");
    }

    void clean() {
        f::remove_all(path());
    }

    void checkInternalFilesCreation() {
        clean();

        // Ensure directory
        f::create_directories(path());


        // Check if UUIDColumn would be correct created with pow2BucketsCountIndex set in range of 1..16.
        for (uint8_t i = 1; i <= 16; ++i) {
            TUUIDColumn column(path(), i);

            // Check if index file and data file are present
            assert(f::exists(path() / f::path(TUUIDColumn::kIndexFilename)));
            assert(f::exists(path() / f::path(TUUIDColumn::kDataFilename)));

            // Clear the directory for next iteration
            f::remove_all(path());
        }

        // UUIDColumn can't be created with pow2BucketsCountIndex set to zero.
        try {
            UUIDColumn column(path(), 0);
            assert(false);
        } catch (...) {
            f::remove_all(path());
        }

        // UUIDColumn can't be created with pow2BucketsCountIndex greater than 16.
        try {
            UUIDColumn column(path(), 17);
            assert(false);
        } catch (...) {
            f::remove_all(path());
        }
    }

    void checkAtomicSet() {
        clean();

        // Ensure directory
        f::create_directories(path());

        NodeUUID u;
        AbstractRecordsHandler::RecordNumber recN = 100;

        {
            // Check saving when column instance is going to destroy
            UUIDColumn column(path(), 2);
            column.set(u, recN, true);
        }
        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u);
            assert(fetchResult.first[0] == 100);
            assert(fetchResult.second == 1);
        }
    }

    void checkNonAtomicSet() {
        clean();

        // Ensure directory
        f::create_directories(path());

        NodeUUID u;
        AbstractRecordsHandler::RecordNumber recN = 100;

        {
            // Check saving when column instance is going to destroy
            UUIDColumn column(path(), 2);
            column.set(u, recN, false);
        }
        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u);

            // Previous set was not atomic.
            // Changes should not be written to the disk.
            assert(fetchResult.second == 0);
        }
    }

    /*
     * This test tries to set one hundred record numbers associated with one uuid.
     */
    void checkSetSeveralRecordNumbersIntoSameRecord() {
        clean();

        // Ensure directory
        f::create_directories(path());

        NodeUUID u;
        {
            UUIDColumn column(path(), 2);
            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<100; recN++) {
                column.set(u, recN, false);
            }
            column.commitCachedBlocks();
        }

        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u);
            assert(fetchResult.second == 100);

            // Record numbers must be sorted in ascending order
            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<100; recN++) {
                assert(fetchResult.first[recN] == recN);
            }
        }


        // Check records numbers inserted in reverse order
        NodeUUID u2;
        {
            UUIDColumn column(path(), 2);
            for (AbstractRecordsHandler::RecordNumber recN = 100; recN>0; recN--) {
                column.set(u2, recN, false);
            }
            column.commitCachedBlocks();
        }
        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u2);
            assert(fetchResult.second == 100);

            // Record numbers must be sorted in ascending order
            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<100; recN++) {
                assert(fetchResult.first[recN] == recN+1);
            }
        }
    }

    void checkAtomicRemoving() {
        clean();
        f::create_directories(path());

        NodeUUID u;
        AbstractRecordsHandler::RecordNumber recN = 100;

        {
            UUIDColumn column(path(), 2);
            column.set(u, recN, true);
            column.remove(u, recN, true);
        }
        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u);
            assert(fetchResult.second == 0);
        }
    }

    /*
     * This test case assigns 100 records to one uuid.
     * Than removes 50 of them and checks if rest 50 are stored correct.
     */
    void checkAtomicRemovingTC2() {
        clean();
        f::create_directories(path());


        NodeUUID u;
        {
            UUIDColumn column(path(), 2);
            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<100; recN++) {
                column.set(u, recN, false);
            }
            column.commitCachedBlocks();


            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<50; recN++) {
                column.remove(u, recN, false);
            }
            column.commitCachedBlocks();
        }

        // Check if rest 50 records are stored correct
        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u);

            // check count == 50
            assert(fetchResult.second == 50);

            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<50; recN++) {
                assert(fetchResult.first[recN] == recN+50);
            }
        }
    }

    void performanceOfWriting() {
        clean();
        f::create_directories(path());


        NodeUUID u;
        {
            UUIDColumn column(path(), 10);

            boost::posix_time::ptime started = boost::posix_time::microsec_clock::local_time();

            const size_t iterationsCount = 100000;
            for (AbstractRecordsHandler::RecordNumber recN = 0; recN<iterationsCount; recN++) {
                column.set(u, recN, false);
            }
            column.commitCachedBlocks();

            boost::posix_time::ptime ended = boost::posix_time::microsec_clock::local_time();
            std::cout << iterationsCount << " insert operations per " << ended-started;
        }
    }

};