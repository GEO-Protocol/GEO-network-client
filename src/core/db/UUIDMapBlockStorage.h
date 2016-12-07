#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H

#include <map>
#include <string>
#include <cstdio>
#include <vector>
#include <malloc.h>
#include <unistd.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "../common/NodeUUID.h"
#include "../common/exceptions/IOError.h"
#include "../common/exceptions/IndexError.h"
#include "../logger/Logger.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;
        
        typedef uint8_t byte;

        class UUIDMapBlockStorage {
        private:
            FILE *mFileDescriptor;
            int mPOSIXFileDescriptor;
            string mFileName;

            uint32_t mMapIndexOffset = 0;
            uint64_t mMapIndexRecordsCount = 0;

            map <NodeUUID, pair<uint32_t, uint64_t>> mIndexBlock;

            const string kTempFileName = "D:/block_storage_temp.dat";
            const string kModeCreate = "w+";
            const string kModeUpdate = "r+";

            const size_t kFileHeaderMapIndexOffset = 4;
            const size_t kFileHeaderMapRecordsCount = 8;
            const size_t kFileHeaderSize = kFileHeaderMapIndexOffset + kFileHeaderMapRecordsCount;
            const size_t kIndexBlockUUIDSize = 16;
            const size_t kIndexBlockOffsetSize = 4;
            const size_t kIndexBlockDataSize = 8;

        public:
            UUIDMapBlockStorage(const string fileName);

            ~UUIDMapBlockStorage();

            void write(const NodeUUID &uuid, const byte *block, const size_t blockBytesCount, const FILE *fileDescriptor);

            void erase(const NodeUUID &uuid);

            Block read(const NodeUUID &uuid);

            const vector <NodeUUID> keys() const;

            void vacuum();

        private:
            void obtainFileDescriptor();

            void checkFileDescriptor();

            void allocateFileHeader();

            void readFileHeader();

            const long writeData(const byte *block, const size_t &blockBytesCount);

            const pair<uint32_t, uint64_t> writeIndexRecordsInMemory(const NodeUUID &uuid, const long &offset, const size_t &blockBytesCount);

            void writeFileHeader();

            void writeIndexBlock();

            void syncData();

            const bool isFileExist();

            const bool isUUIDTheIndex(const NodeUUID &uuid);
        };

        class UUIDMapBlockStorage;

        class Block {
            friend class UUIDMapBlockStorage;

        private:
            byte *mData;
            size_t mBytesCount;

        public:
            Block(const byte *data, const size_t bytesCount);

            ~Block();

            const byte *data() const;

            const size_t bytesCount() const;
        };
    }
}
#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
