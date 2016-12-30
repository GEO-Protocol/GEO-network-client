#include "UUIDMapBlockStorage.h"

namespace db {
    namespace uuid_map_block_storage {

        UUIDMapBlockStorage::UUIDMapBlockStorage(const string &directory, const string &fileName) :
                mDirectory(directory),
                mFileName(fileName),
                mTempFileName(string("temp_") + mFileName),
                mFilePath(mDirectory + "/" + mFileName),
                mTempFilePath(mDirectory + "/" + mTempFileName){
            
            removeTemporaryFile();
            obtainFileDescriptor();
            readFileHeader();
            readIndexBlock();
        }

        UUIDMapBlockStorage::~UUIDMapBlockStorage() {
            if (mFileDescriptor != NULL) {
                fclose(mFileDescriptor);
            }
        }

        void UUIDMapBlockStorage::write(const uuids::uuid &uuid, const byte *block, const size_t blockBytesCount) {
            if (isUUIDTheIndex(uuid)){
                throw ConflictError("UUIDMapBlockStorage::write. Can't write data. Index with such uuid already exist. Maybe you should to use rewrite() method.");
            }
            long offset = writeData(block, blockBytesCount);
            pair<uint32_t, uint64_t> fileHeaderData = writeIndexRecordsInMemory(uuid, offset, blockBytesCount);
            mMapIndexOffset = fileHeaderData.first;
            mMapIndexRecordsCount = fileHeaderData.second;
            writeFileHeader();
        }

        void UUIDMapBlockStorage::rewrite(const uuids::uuid &uuid, const byte *block, const size_t blockBytesCount) {
            if (!isUUIDTheIndex(uuid)){
                throw ConflictError("UUIDMapBlockStorage::rewrite. Unable to rewrite data. Index with such uuid does not exist. Maybe you to should use write() method.");
            }
            erase(uuid);
            long offset = writeData(block, blockBytesCount);
            pair<uint32_t, uint64_t> fileHeaderData = writeIndexRecordsInMemory(uuid, offset, blockBytesCount);
            mMapIndexOffset = fileHeaderData.first;
            mMapIndexRecordsCount = fileHeaderData.second;
            writeFileHeader();
        }

        void UUIDMapBlockStorage::erase(const uuids::uuid &uuid) {
            if (isUUIDTheIndex(uuid)){
                map<uuids::uuid, pair<uint32_t, uint64_t>>::iterator seeker = mIndexBlock.find(uuid);
                if (seeker != mIndexBlock.end()) {
                    mIndexBlock.erase(seeker);
                    mMapIndexRecordsCount = (uint64_t) mIndexBlock.size();
                    fseek(mFileDescriptor, 0, SEEK_END);
                    mMapIndexOffset = (uint32_t) ftell(mFileDescriptor);
                    writeIndexBlock();
                    writeFileHeader();
                } else {
                    throw IndexError("UUIDMapBlockStorage::erase. Can't iterate map to find value by such key.");
                }
            } else {
                throw IndexError("UUIDMapBlockStorage::erase. Can't find such uuid in index block.");
            }
        }

        Record *UUIDMapBlockStorage::readFromFile(const uuids::uuid &uuid) {
            if (isUUIDTheIndex(uuid)){
                map<uuids::uuid, pair<uint32_t, uint64_t>>::iterator seeker = mIndexBlock.find(uuid);
                if (seeker != mIndexBlock.end()) {
                    size_t offset = (size_t) seeker->second.first;
                    size_t bytesCount = (size_t) seeker->second.second;
                    fseek(mFileDescriptor, offset, SEEK_SET);
                    byte *dataBuffer = (byte *)malloc(bytesCount);
                    memset(dataBuffer, 0, bytesCount);
                    if (fread(dataBuffer, 1, bytesCount, mFileDescriptor) != bytesCount) {
                        if (fread(dataBuffer, 1, bytesCount, mFileDescriptor) != bytesCount) {
                            free(dataBuffer);
                            throw IOError("UUIDMapBlockStorage::readFromFile. Can't read data block from file in buffer.");
                        }
                    }
                    return new Record(dataBuffer, bytesCount);
                } else {
                    throw IndexError("UUIDMapBlockStorage::readFromFile. Can't iterate map to find value by such key.");
                }
            } else {
                throw IndexError("UUIDMapBlockStorage::readFromFile. Can't find such uuid in index block.");
            }
        }

        const vector <uuids::uuid> UUIDMapBlockStorage::keys() const {
            vector<uuids::uuid> uuidsVector;
            uuidsVector.reserve(mIndexBlock.size());
            for (auto const &it : mIndexBlock){
                uuidsVector.push_back(it.first);
            }
            return uuidsVector;
        }

        void UUIDMapBlockStorage::vacuum() {
            UUIDMapBlockStorage *mapBlockStorage = new UUIDMapBlockStorage(mDirectory, mTempFileName);
            map<uuids::uuid, pair<uint32_t, uint64_t>>::iterator looper;
            for (looper = mIndexBlock.begin(); looper != mIndexBlock.end(); ++looper){
                Record *block = readFromFile(looper->first);
                mapBlockStorage->write(looper->first, block->mData, block->mBytesCount);
                delete block;
            }
            delete mapBlockStorage;
            fclose(mFileDescriptor);
            if (remove(mFilePath.c_str()) != 0){
                throw IOError("UUIDMapBlockStorage::vacuum. Can't remove old *.bin file.");
            }
            if (rename(mTempFilePath.c_str(), mFilePath.c_str()) != 0){
                throw IOError("UUIDMapBlockStorage::vacuum. Can't rename temporary *.bin file to main *.bin file.");
            }
            obtainFileDescriptor();
        }

        void UUIDMapBlockStorage::obtainFileDescriptor() {
            if (isFileExist(mFilePath)) {
                mFileDescriptor = fopen(mFilePath.c_str(), kModeUpdate.c_str());
                checkFileDescriptor();
            } else {
                mFileDescriptor = fopen(mFilePath.c_str(), kModeCreate.c_str());
                checkFileDescriptor();
                allocateFileHeader();
            }
        }

        void UUIDMapBlockStorage::checkFileDescriptor() {
            if (mFileDescriptor == NULL) {
                throw IOError("UUIDMapBlockStorage::checkFileDescriptor. Unable to obtain file descriptor.");
            }
            mPOSIXFileDescriptor = fileno(mFileDescriptor);
        }

        void UUIDMapBlockStorage::allocateFileHeader() {
            byte *buffer = (byte *) malloc(kFileHeaderSize);
            memset(buffer, 0, kFileHeaderSize);
            fseek(mFileDescriptor, 0, SEEK_SET);
            if (fwrite(buffer, 1, kFileHeaderSize, mFileDescriptor) != kFileHeaderSize) {
                if (fwrite(buffer, 1, kFileHeaderSize, mFileDescriptor) != kFileHeaderSize) {
                    free(buffer);
                    throw IOError("UUIDMapBlockStorage::allocateFileHeader. Can't allocate default empty file header.");
                }
            }
            free(buffer);
            syncData();
        }

        void UUIDMapBlockStorage::readFileHeader() {
            checkFileDescriptor();
            byte *headerBuffer = (byte *) malloc(kFileHeaderMapIndexOffset);
            memset(headerBuffer, 0, kFileHeaderMapIndexOffset);
            fseek(mFileDescriptor, 0, SEEK_SET);
            if (fread(headerBuffer, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                if (fread(headerBuffer, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                    free(headerBuffer);
                    throw IOError("UUIDMapBlockStorage::readFileHeader. Can't read index block offset from file header");
                }
            }
            mMapIndexOffset = *((uint32_t *) headerBuffer);
            free(headerBuffer);
            headerBuffer = (byte *)malloc(kFileHeaderMapRecordsCount);
            memset(headerBuffer, 0, kFileHeaderMapRecordsCount);
            if (fread(headerBuffer, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount) {
                if (fread(headerBuffer, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount) {
                    free(headerBuffer);
                    throw IOError("UUIDMapBlockStorage::readFileHeader. Can't read index records count in index block from file header");
                }
            }
            mMapIndexRecordsCount = *((uint64_t *) headerBuffer);
            free(headerBuffer);
        }

        void UUIDMapBlockStorage::readIndexBlock() {
            if (mMapIndexRecordsCount > 0) {
                byte *indexBlockBuffer = (byte *) malloc(kIndexRecordSize * mMapIndexRecordsCount);
                memset(indexBlockBuffer, 0, kIndexRecordSize * mMapIndexRecordsCount);
                fseek(mFileDescriptor, mMapIndexOffset, SEEK_SET);
                if (fread(indexBlockBuffer, 1, kIndexRecordSize * mMapIndexRecordsCount, mFileDescriptor) != kIndexRecordSize * mMapIndexRecordsCount){
                    if (fread(indexBlockBuffer, 1, kIndexRecordSize * mMapIndexRecordsCount, mFileDescriptor) != kIndexRecordSize * mMapIndexRecordsCount){
                        free(indexBlockBuffer);
                        throw IOError("UUIDMapBlockStorage::readIndexBlock. Error while reading index block from *.bin file.");
                    }
                }

                byte *indexBlockBufferOffset = indexBlockBuffer;
                for (size_t recordNumber = 0; recordNumber < mMapIndexRecordsCount; ++ recordNumber) {
                    uuids::uuid uuid;
                    memcpy(&uuid, indexBlockBufferOffset, 16);
                    uint32_t *offset = new (indexBlockBufferOffset + kIndexRecordUUIDSize) uint32_t;
                    uint64_t *blockBytesCount = new (indexBlockBufferOffset + kIndexRecordUUIDSize + sizeof(uint32_t)) uint64_t;
                    mIndexBlock.insert(pair<uuids::uuid, pair<uint32_t, uint64_t>>(uuid, make_pair(*offset, *blockBytesCount)));
                    indexBlockBufferOffset += kIndexRecordSize;
                }
                free(indexBlockBuffer);
            }
        }

        const long UUIDMapBlockStorage::writeData(const byte *block, const size_t blockBytesCount) {
            fseek(mFileDescriptor, 0, SEEK_END);
            long offset = ftell(mFileDescriptor);
            if (fwrite(block, 1, blockBytesCount, mFileDescriptor) != blockBytesCount) {
                if (fwrite(block, 1, blockBytesCount, mFileDescriptor) != blockBytesCount) {
                    throw IOError("UUIDMapBlockStorage::writeData. Can't write data buffer in file.");
                }
            }
            syncData();
            return offset;
        }

        const pair<uint32_t, uint64_t> UUIDMapBlockStorage::writeIndexRecordsInMemory(const uuids::uuid &uuid, const long offset,
                                                          const size_t blockBytesCount) {
            fseek(mFileDescriptor, 0, SEEK_END);
            long offsetToIndexRecords = ftell(mFileDescriptor);
            mIndexBlock.insert(pair<uuids::uuid, pair<uint32_t, uint64_t>>(uuid, make_pair((uint32_t) offset, (uint64_t) blockBytesCount)));
            size_t recordsCount = mIndexBlock.size();
            writeIndexBlock();
            return make_pair((uint32_t) offsetToIndexRecords, (uint64_t) recordsCount);
        }

        void UUIDMapBlockStorage::writeFileHeader() {
            fseek(mFileDescriptor, 0, SEEK_SET);
            if (fwrite(&mMapIndexOffset, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                if (fwrite(&mMapIndexOffset, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                    throw IOError("UUIDMapBlockStorage::writeFileHeader. Can't write index block offset in file header.");
                }
            }
            if (fwrite(&mMapIndexRecordsCount, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount){
                if (fwrite(&mMapIndexRecordsCount, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount){
                    throw IOError("UUIDMapBlockStorage::writeFileHeader. Can't write index records count in index block in file header.");
                }
            }
            syncData();
        }

        void UUIDMapBlockStorage::writeIndexBlock() {
            map<uuids::uuid, pair<uint32_t, uint64_t>>::iterator looper;
            for (looper = mIndexBlock.begin(); looper != mIndexBlock.end(); ++looper){
                if (fwrite(looper->first.data, 1, kIndexRecordUUIDSize, mFileDescriptor) != kIndexRecordUUIDSize){
                    if (fwrite(looper->first.data, 1, kIndexRecordUUIDSize, mFileDescriptor) != kIndexRecordUUIDSize){
                        throw IOError("UUIDMapBlockStorage::writeIndexBlock. Can't uuid in index block.");
                    }
                }
                if (fwrite(&looper->second.first, 1, kIndexRecordOffsetSize, mFileDescriptor) != kIndexRecordOffsetSize){
                    if (fwrite(&looper->second.first, 1, kIndexRecordOffsetSize, mFileDescriptor) != kIndexRecordOffsetSize){
                        throw IOError("UUIDMapBlockStorage::writeIndexBlock. Can't offset to data block in index block.");
                    }
                }
                if (fwrite(&looper->second.second, 1, kIndexRecordDataSize, mFileDescriptor) != kIndexRecordDataSize){
                    if (fwrite(&looper->second.second, 1, kIndexRecordDataSize, mFileDescriptor) != kIndexRecordDataSize){
                        throw IOError("UUIDMapBlockStorage::writeIndexBlock. Can't data bytes count in index block.");
                    }
                }
            }
            syncData();
        }

        void UUIDMapBlockStorage::syncData() {
            fdatasync(mPOSIXFileDescriptor);
        }

        void UUIDMapBlockStorage::removeTemporaryFile() {
            if (isFileExist(mFilePath)){
                if (isFileExist(mTempFilePath)){
                    if (remove(mTempFilePath.c_str()) != 0){
                        throw IOError("UUIDMapBlockStorage::removeTemporaryFile. Can't remove old *.bin file.");
                    }
                }
            } else {
                if (isFileExist(mTempFilePath)){
                    if (rename(mTempFilePath.c_str(), mFilePath.c_str()) != 0){
                        throw IOError("UUIDMapBlockStorage::removeTemporaryFile. Can't rename temporary *.bin file to main *.bin file.");
                    }
                }
            }
        }

        const bool UUIDMapBlockStorage::isFileExist(string &fileName) {
            return fs::exists(fs::path(fileName.c_str()));
        }

        const bool UUIDMapBlockStorage::isUUIDTheIndex(const uuids::uuid &uuid) {
            return mIndexBlock.count(uuid) > 0;
        }


    }
}

