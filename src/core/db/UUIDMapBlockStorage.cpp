#include "UUIDMapBlockStorage.h"

namespace db {
    namespace uuid_map_block_storage {

        UUIDMapBlockStorage::UUIDMapBlockStorage() {
            obtainFileDescriptor();
        }

        UUIDMapBlockStorage::~UUIDMapBlockStorage() {
            if (mFileDescriptor != NULL) {
                fclose(mFileDescriptor);
            }
        }

        void UUIDMapBlockStorage::write(const uuids::uuid &uuid, const byte *block, const size_t blockBytesCount) {
            long offset = writeData(block, blockBytesCount);
            mMapIndexOffset = (uint32_t) writeIndexRecords(uuid, offset, blockBytesCount);
        }

        void UUIDMapBlockStorage::erase(const uuids::uuid &uuid) {}

        Block UUIDMapBlockStorage::read(const uuids::uuid &uuid) {}

        const vector <uuids::uuid> UUIDMapBlockStorage::keys() const {}

        void UUIDMapBlockStorage::vacuum() {}

        void UUIDMapBlockStorage::obtainFileDescriptor() {
            if (isFileExist()) {
                mFileDescriptor = fopen(kFileName.c_str(), kModeUpdate.c_str());
            } else {
                mFileDescriptor = fopen(kFileName.c_str(), kModeCreate.c_str());
                allocateFileHeader();
            }
        }

        void UUIDMapBlockStorage::checkFileDescriptor() {
            if (mFileDescriptor == NULL) {
                mLogger.logInfo(kSubsystemName, "Unable to obtain *.dat file descriptor");
                throw IOError("Unable to obtain file descriptor");
            }
        }

        void UUIDMapBlockStorage::allocateFileHeader() {
            byte *buffer = (byte *) malloc(kFileHeaderSize);
            memset(buffer, 0, kFileHeaderSize);
            fseek(mFileDescriptor, 0, SEEK_SET);
            fwrite(buffer, 1, kFileHeaderSize, mFileDescriptor);
            mFileSize += kFileHeaderSize;
        }

        void UUIDMapBlockStorage::readFileHeader() {
            //Get old map index offset and record count from file header
            byte *headerBuffer = malloc(kFileHeaderMapIndexOffset);
            memset(headerBuffer, 0, kFileHeaderMapIndexOffset);
            //Seek to start of file
            fseek(mFileDescriptor, 0, SEEK_SET);
            //Read map index offset that takes 4 bytes and write in buffer
            fread(headerBuffer, 1, kFileHeaderMapIndexOffset, mFileDescriptor);
            //Convert map index offset bytes buffer to numeric value
            mMapIndexOffset = *((uint32_t *) headerBuffer);
            //Free allocated memory for headerBuffer further using
            free(headerBuffer);
            //Reallocate buffer
            headerBuffer = malloc(kFileHeaderMapRecordsCount);
            memset(headerBuffer, 0, kFileHeaderMapRecordsCount);
            //Read map records count that takes 8 bytes and write in buffer
            fread(headerBuffer, 1, kFileHeaderMapRecordsCount, mFileDescriptor);
            //Convert map records count bytes buffer to numeric value
            mMapIndexRecordsCount = *((uint64_t *) headerBuffer);
            //Free allocated memory
            free(headerBuffer);
        }

        const bool UUIDMapBlockStorage::isFileExist() {
            File *file = fopen(kFileName.c_str(), "w");
            if (file != NULL) {
                fclose(file);
                return true;
            }
            return false;
        }

        const long UUIDMapBlockStorage::writeData(const byte *block, const size_t &blockBytesCount) {
            //Seek to end of file
            fseek(mFileDescriptor, 0, SEEK_END);
            //Get offset (pointer to data block) before start writing
            long offset = ftell(mFileDescriptor);
            //Write block with data
            fwrite(block, 1, blockBytesCount, mFileDescriptor);
            return offset;
        }

        const long UUIDMapBlockStorage::writeIndexRecords(const uuids::uuid &uuid, const long &offset,
                                                          const size_t &blockBytesCount) {
            //Seek to end of file
            fseek(mFileDescriptor, 0, SEEK_END);
            //Get offset (pointer to first index record) of index blocks
            long offsetToIndexBlocks = ftell(mFileDescriptor);
            //Insert data about new index record into the map
            if (mIndexBlock.count(uuid) > 0) {
                map<uuids::uuid, pair<uint32_t, uint64_t>>::iterator seeker = mIndexBlock.find(uuid);
                if (seeker != mIndexBlock.end()) {
                    seeker->second = make_pair((uint32_t) offset, (uint64_t) blockBytesCount);
                }
            } else {
                mIndexBlock.insert(uuids::uuid, pair<uint32_t, uint64_t>(uuid, make_pair((uint32_t) offset, (uint64_t) blockBytesCount)));
            }
            //Iterate through map and create index block from index records
            //after last data block
            map<uuids::uuid, pair<uint32_t, uint64_t>>::iterator looper;
            for (looper = mIndexBlock.begin(); looper != mIndexBlock.end(); ++it){
                //Write uuid to the index record
                fwrite(looper->first, 1, kIndexBlockUUIDSize, mFileDescriptor);
                //Write offset to the index record
                fwrite(looper->first->first, 1, kIndexBlockOffsetSize, mFileDescriptor);
                //Write data block size to the index record
                fwrite(looper->first->second, 1, kIndexBlockDataSize, mFileDescriptor);
            }
            return offsetToIndexBlocks;
        }

        void UUIDMapBlockStorage::updateFileHeader() {
            //Seek to start of file and write in file header map index offset and records count
            fseek(mFileDescriptor, 0, SEEK_SET);
            //Write index offset in file header
            fwrite(&mMapIndexOffset, 1, kFileHeaderMapIndexOffset, mFileDescriptor);
            //Write record count in file header
            fwrite(&mMapIndexRecordsCount, 1, kFileHeaderMapRecordsCount, mFileDescriptor);
        }

        const byte *Block::data() const {
            return mData;
        }

        const size_t Block::bytesCount() const {
            return mBytesCount;
        }

    }
}

