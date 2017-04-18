#ifndef GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
#define GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H

#include "../../common/Types.h"
#include "Record.h"

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

#include <map>
#include <string>
#include <cstdio>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

#include "../../common/exceptions/IOError.h"
#include "../../common/exceptions/IndexError.h"
#include "../../common/exceptions/MemoryError.h"
#include "../../common/exceptions/ConflictError.h"

namespace db {
    namespace uuid_map_block_storage {

        using namespace std;

        namespace fs = boost::filesystem;
        namespace uuids = boost::uuids;

        class UUIDMapBlockStorageTest;

        class UUIDMapBlockStorage {
            friend class UUIDMapBlockStorageTest;

        public:
            UUIDMapBlockStorage(
                const string &directory,
                const string &fileName);

            ~UUIDMapBlockStorage();

            void write(
                const uuids::uuid &uuid,
                const byte *record,
                const size_t blockBytesCount);

            void rewrite(const uuids::uuid &uuid,
                         const byte *block,
                         const size_t blockBytesCount);

            void erase(
                const uuids::uuid &uuid);

            Record::Shared readByUUID(
                const uuids::uuid &uuid);

            const vector<uuids::uuid>* keys() const;

            const bool isExist(
                const uuids::uuid &uuid);

            void vacuum();

        private:
            void checkDirectory();

            void removeTemporaryFile();

            void obtainFileDescriptor();

            void checkFileDescriptor();

            void allocateFileHeader();

            void readFileHeader();

            void readIndexBlock();

            const long writeData(
                const byte *block,
                const size_t blockBytesCount);

            const pair<uint32_t, uint64_t> writeIndexRecordsInMemory(
                const uuids::uuid &uuid,
                const long offset,
                const size_t blockBytesCount);

            void writeIndexBlock();

            void writeFileHeader();

            void syncData();

            const bool isFileExist(
                string &fileName);

            const bool isUUIDTheIndex(
                const uuids::uuid &uuid);

        private:
            string mDirectory;
            string mFileName;
            string mFilePath;
            string mTempFileName;
            string mTempFilePath;

            FILE *mFileDescriptor;
            int mPOSIXFileDescriptor;

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
        };
    }
}
#endif //GEO_NETWORK_CLIENT_UUIDMAPBLOCKSTORAGE_H
