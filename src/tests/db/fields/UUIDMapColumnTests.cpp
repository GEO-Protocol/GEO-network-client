#include "../../../core/db/fields/uuid_column/UUIDColumn.h"


using namespace db;
using namespace db::fields;


namespace f = boost::filesystem;


class UUIDMapColumnTests {
public:
    void run() {
//        checkInternalFilesCreation();
        checkAtomicSet();

        clean();
    };

protected:
    f::path path() {
        return f::path("tests/db/trust_line_direction_column/");
    }

    void clean() {
        f::remove_all(path());
    }

    void checkInternalFilesCreation() {
        clean();

        // Ensure directory
        f::create_directories(path());

        auto dataFilename = (path() / f::path("data.bin"));
        auto indexFilename = (path() / f::path("data.bin"));

        for (uint8_t i = 1; i <= 16; ++i) {
            UUIDColumn column(path(), i);

            assert(f::exists(dataFilename));
            assert(f::file_size(dataFilename) > 0);

            assert(f::exists(indexFilename));
            assert(f::file_size(indexFilename) > 0);

            // Clear the directory
            f::remove_all(path());
        }

        // Check creation with 0 buckets index
        try {
            UUIDColumn column(path(), 0);
            assert(false);
        } catch (...) {
            // Clear the directory
            f::remove_all(path());
        }


        // Check creation with 17 buckets index
        try {
            UUIDColumn column(path(), 17);
            assert(false);
        } catch (...) {
            // Clear the directory
            f::remove_all(path());
        }
    }

    void checkAtomicSet() {
        clean();

        // Ensure directory
        f::create_directories(path());
        auto dataFilename = (path() / f::path("data.bin"));
        auto indexFilename = (path() / f::path("data.bin"));


        NodeUUID u;
        AbstractRecordsHandler::RecordNumber recN = 1;

        {
            // Check saving when column instance is going to destroy
            UUIDColumn column(path(), 2);
            column.set(u, recN, true);
        }
        {
            UUIDColumn column(path(), 2);
            auto fetchResult = column.recordNumbersAssignedToUUID(u);
            assert(fetchResult.second == 1);
        }
    }
};