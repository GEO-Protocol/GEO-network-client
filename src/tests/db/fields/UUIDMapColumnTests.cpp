#include "../../../core/db/fields/uuid_column/UUIDColumn.h"


using namespace db;
using namespace db::fields;


namespace f = boost::filesystem;


class UUIDColumnTests {
public:
    void run() {
//        checkInternalFilesCreation();
        checkAtomicSet();


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
};