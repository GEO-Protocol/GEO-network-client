#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H

#include <map>
#include <string>
#include <cstdio>
#include <vector>
#include <malloc.h>
#include <unistd.h>

#include <boost/filesystem.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include "Block.h"

#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/IndexError.h"
#include "../../common/exceptions/ConflictError.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        namespace fs = boost::filesystem;
        namespace uuids = boost::uuids;
        
        typedef uint8_t byte;

        class UUIDMapBlockStorageTest;

        class UUIDMapBlockStorage {
            friend class UUIDMapBlockStorageTest;

        private:
            FILE *mFileDescriptor;
            int mPOSIXFileDescriptor;
            string mDirectory;
            string mFileName;
            string mTempFileName;
            string mFilePath;
            string mTempFilePath;

            uint32_t mMapIndexOffset = 0;
            uint64_t mMapIndexRecordsCount = 0;

            map <uuids::uuid, pair<uint32_t, uint64_t>> mIndexBlock;

            const string kModeCreate = "w+";
            const string kModeUpdate = "r+";

            const size_t kFileHeaderMapIndexOffset = 4;
            const size_t kFileHeaderMapRecordsCount = 8;
            const size_t kFileHeaderSize = kFileHeaderMapIndexOffset + kFileHeaderMapRecordsCount;
            const size_t kIndexRecordUUIDSize = 16;
            const size_t kIndexRecordOffsetSize = 4;
            const size_t kIndexRecordDataSize = 8;
            const size_t kIndexRecordSize = kIndexRecordUUIDSize + kIndexRecordOffsetSize + kIndexRecordDataSize;

        public:
            UUIDMapBlockStorage(const string &directory, const string &fileName);

            ~UUIDMapBlockStorage();

            void write(const uuids::uuid &uuid, const byte *block, const size_t blockBytesCount);

            void rewrite(const uuids::uuid &uuid, const byte *block, const size_t blockBytesCount);

            void erase(const uuids::uuid &uuid);

            Block *readFromFile(const uuids::uuid &uuid);

            const vector <uuids::uuid> keys() const;

            void vacuum();

        private:
            void obtainFileDescriptor();

            void checkFileDescriptor();

            void allocateFileHeader();

            void readFileHeader();

            void readIndexBlock();

            const long writeData(const byte *block, const size_t blockBytesCount);

            const pair<uint32_t, uint64_t> writeIndexRecordsInMemory(const uuids::uuid &uuid, const long offset, const size_t blockBytesCount);

            void writeFileHeader();

            void writeIndexBlock();

            void syncData();

            void removeTemporaryFile();

            const bool isFileExist(string &fileName);

            const bool isUUIDTheIndex(const uuids::uuid &uuid);
        };

    }
}
#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
