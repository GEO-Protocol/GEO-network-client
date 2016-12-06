#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H

#include <map>
#include <string>
#include <stdio>
#include <vector>
#include <malloc.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "../common/exceptions/IOError.h"
#include "../logger/Logger.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        namespace uuids = boost::uuids;

        typedef uint8_t byte;

        class UUIDMapBlockStorage {
        public:
            Logger mLogger;
            const constexpr string kSubsystemName = "UUIDMapBlockStorage";

        private:
            File *mFileDescriptor;

            uint32_t mMapIndexOffset = 0;
            uint64_t mMapIndexRecordsCount = 0;

            map <uuids::uuid, pair<uint32_t, uint64_t>> mIndexBlock;

            const constexpr string kFileName = "D:/block_storage.dat";
            const constexpr string kModeCreate = "w+";
            const constexpr string kModeUpdate = "r+";

            const size_t kFileHeaderMapIndexOffset = 4;
            const size_t kFileHeaderMapRecordsCount = 8;
            const size_t kFileHeaderSize = kFileHeaderMapIndexOffset + kFileHeaderMapRecordsCount;
            const size_t kIndexBlockUUIDSize = 16;
            const size_t kIndexBlockOffsetSize = 4;
            const size_t kIndexBlockDataSize = 8;
            const size_t kIndexBlockSize = kIndexBlockUUIDSize + kIndexBlockOffsetSize + kIndexBlockDataSize;

        public:
            UUIDMapBlockStorage();

            ~UUIDMapBlockStorage();

            void write(const uuids::uuid &uuid, const byte *block, const size_t blockBytesCount);

            void erase(const uuids::uuid &uuid);

            Block read(const uuids::uuid &uuid);

            const vector <uuids::uuid> keys() const;

            void vacuum();

        private:
            void obtainFileDescriptor();

            void checkFileDescriptor();

            void allocateFileHeader();

            void readFileHeader();

            const long writeData(const byte *block, const size_t &blockBytesCount);

            const long writeIndexRecords(const uuids::uuid &uuid, const long &offset, const size_t &blockBytesCount);

            void updateFileHeader(const size_t offsetToIndexBlock);

            const bool isFileExist();
        };

        class UUIDMapBlockStorage;

        class Block {
            friend class UUIDMapBlockStorage;

        private:
            byte *mData;
            size_t mBytesCount;

        public:
            const byte *data() const;

            const size_t bytesCount() const;
        };
    }
}
#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
