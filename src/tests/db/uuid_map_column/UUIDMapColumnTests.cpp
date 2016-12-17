#include "../../../core/db/fields/uuid_map_column/UUIDMapColumn.h"

#include <boost/filesystem.hpp>


namespace db {
namespace fields {


namespace fs = boost::filesystem;


class UUIDMapColumnTests {
public:
    static const constexpr char *path = "tests/db/uuid_map_column/";

public:
    void run() {
//        checkInternalFilesCreation();
        checkAtomicSet();
    };

    void checkInternalFilesCreation() {

        // Ensure directory
        fs::create_directories(fs::path(path));

        auto dataFilename = fs::path(string(path) + "data.bin");
        auto indexFilename = fs::path(string(path) + "data.bin");

        for (uint8_t i = 1; i <= 16; ++i) {
            UUIDMapColumn column(path, i);

            assert(fs::exists(dataFilename));
            assert(fs::file_size(dataFilename) > 0);

            assert(fs::exists(indexFilename));
            assert(fs::file_size(indexFilename) > 0);

            // Clear the directory
            fs::remove_all(fs::path(path));
        }

        // Check creation with 0 buckets index
        try {
            UUIDMapColumn column(path, 0);
            assert(false);
        } catch (...) {
            // Clear the directory
            fs::remove_all(fs::path(path));
        }


        // Check creation with 17 buckets index
        try {
            UUIDMapColumn column(path, 17);
            assert(false);
        } catch (...) {
            // Clear the directory
            fs::remove_all(fs::path(path));
        }
    }

    void checkAtomicSet() {
        // Ensure directory
        fs::create_directories(fs::path(path));

        auto dataFilename = fs::path(string(path) + "data.bin");
        auto indexFilename = fs::path(string(path) + "data.bin");

        UUIDMapColumn column(path, 2);

        NodeUUID u;
        AbstractRecordsHandler::RecordNumber recN = 1;

        column.set(u, recN, true);


//        // Clear the directory
//        fs::remove_all(fs::path(path));
    }

};


}
}

