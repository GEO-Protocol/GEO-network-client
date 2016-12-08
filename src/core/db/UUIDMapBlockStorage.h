#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H

#include <map>
#include <string>
#include <cstdio>
#include <vector>
#include <malloc.h>
#include <unistd.h>
#include "Block.h"
#include <boost/filesystem.hpp>
#include "../common/NodeUUID.h"
#include "../common/exceptions/IOError.h"
#include "../common/exceptions/IndexError.h"
#include "../common/exceptions/ConflictError.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        namespace fs = boost::filesystem;
        
        typedef uint8_t byte;

        class UUIDMapBlockStorage {
        private:
            FILE *mFileDescriptor;
            int mPOSIXFileDescriptor;
            string mFileName;

            uint32_t mMapIndexOffset = 0;
            uint64_t mMapIndexRecordsCount = 0;

            map <NodeUUID, pair<uint32_t, uint64_t>> mIndexBlock;

            const string kTempFileName = "block_storage_temp.bin";
            const string kModeCreate = "w+";
            const string kModeUpdate = "r+";

            const size_t kFileHeaderMapIndexOffset = 4;
            const size_t kFileHeaderMapRecordsCount = 8;
            const size_t kFileHeaderSize = kFileHeaderMapIndexOffset + kFileHeaderMapRecordsCount;
            const size_t kIndexRecordUUIDSize = 16;
            const size_t kIndexRecordOffsetSize = 4;
            const size_t kIndexRecordDataSize = 8;
            const size_t kIndexRecordSize = kIndexRecordUUIDSize + kIndexRecordOffsetSize + kIndexBlockDataSize;

        public:
            UUIDMapBlockStorage(const string fileName);

            ~UUIDMapBlockStorage();

            void write(const NodeUUID &uuid, const byte *block, const size_t blockBytesCount);

            void rewrite(const NodeUUID &uuid, const byte *block, const size_t blockBytesCount);

            void erase(const NodeUUID &uuid);

            Block readFromFile(const NodeUUID &uuid);

            const vector <NodeUUID> keys() const;

            void vacuum();

        private:
            void obtainFileDescriptor();

            void checkFileDescriptor();

            void allocateFileHeader();

            void readFileHeader();

            void readIndexBlock();

            const long writeData(const byte *block, const size_t blockBytesCount);

            const pair<uint32_t, uint64_t> writeIndexRecordsInMemory(const NodeUUID &uuid, const long offset, const size_t blockBytesCount);

            void writeFileHeader();

            void writeIndexBlock();

            void syncData();

            const bool isFileExist();

            const bool isUUIDTheIndex(const NodeUUID &uuid);
        };

    }
}
#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
