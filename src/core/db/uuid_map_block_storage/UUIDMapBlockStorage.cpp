#include "UUIDMapBlockStorage.h"

namespace db {
    namespace uuid_map_block_storage {

        UUIDMapBlockStorage::UUIDMapBlockStorage(
            const string &directory,
            const string &fileName) :

            mDirectory(directory),
            mFileName(fileName),
            mFilePath(mDirectory + "/" + mFileName),
            mTempFileName(string("temp_") + mFileName),
            mTempFilePath(mDirectory + "/" + mTempFileName) {

            checkDirectory();
            removeTemporaryFile();
            obtainFileDescriptor();
            readFileHeader();
            readIndexBlock();
        }

        UUIDMapBlockStorage::~UUIDMapBlockStorage() {

            fclose(mFileDescriptor);
        }

        void UUIDMapBlockStorage::write(
            const uuids::uuid &uuid,
            const byte *record,
            const size_t blockBytesCount) {

            if (isUUIDTheIndex(uuid)){
                throw ConflictError("UUIDMapBlockStorage::write. "
                                        "Can't write data. Index with such transactionUUID already exist. Maybe you should to use rewrite() method.");
            }

            long offset = writeData(
                record,
                blockBytesCount
            );
            auto fileHeaderData = writeIndexRecordsInMemory(
                uuid,
                offset,
                blockBytesCount
            );
            mMapIndexOffset = fileHeaderData.first;
            mMapIndexRecordsCount = fileHeaderData.second;
            writeFileHeader();
        }

        void UUIDMapBlockStorage::rewrite(
            const uuids::uuid &uuid,
            const byte *block,
            const size_t blockBytesCount) {

            if (!isUUIDTheIndex(uuid)){
                throw ConflictError("UUIDMapBlockStorage::rewrite. "
                                        "Unable to rewrite data. Index with such transactionUUID does not exist. Maybe you to should use write() method.");
            }

            erase(uuid);
            long offset = writeData(
                block,
                blockBytesCount
            );
            auto fileHeaderData = writeIndexRecordsInMemory(
                uuid,
                offset,
                blockBytesCount
            );
            mMapIndexOffset = fileHeaderData.first;
            mMapIndexRecordsCount = fileHeaderData.second;
            writeFileHeader();
        }

        void UUIDMapBlockStorage::erase(
            const uuids::uuid &uuid) {

            if (isUUIDTheIndex(uuid)){
                auto seeker = mIndexBlock.find(uuid);
                mIndexBlock.erase(seeker);
                mMapIndexRecordsCount = (uint64_t) mIndexBlock.size();
                fseek(
                    mFileDescriptor,
                    0,
                    SEEK_END
                );
                mMapIndexOffset = (uint32_t) ftell(mFileDescriptor);
                writeIndexBlock();
                writeFileHeader();

            } else {
                throw IndexError("UUIDMapBlockStorage::erase. "
                                     "Can't find such transactionUUID in index block.");
            }
        }

        Record::Shared UUIDMapBlockStorage::readByUUID(
            const uuids::uuid &uuid) {

            if (isUUIDTheIndex(uuid)){
                auto seeker = mIndexBlock.find(uuid);
                size_t offset = (size_t) seeker->second.first;
                size_t bytesCount = (size_t) seeker->second.second;
                fseek(
                    mFileDescriptor,
                    offset,
                    SEEK_SET
                );
                byte *dataBuffer = (byte *) malloc(bytesCount);
                memset(
                    dataBuffer,
                    0,
                    bytesCount
                );
                if (fread(dataBuffer, 1, bytesCount, mFileDescriptor) != bytesCount) {
                    if (fread(dataBuffer, 1, bytesCount, mFileDescriptor) != bytesCount) {
                        free(dataBuffer);
                        throw IOError("UUIDMapBlockStorage::readByUUID. "
                                          "Can't read data block from file in buffer.");
                    }
                }
                Record *record = nullptr;
                try {
                    record = new Record(dataBuffer, bytesCount);

                } catch(std::bad_alloc &e) {
                    free(dataBuffer);
                    throw MemoryError("UUIDMapBlockStorage::readByUUID. "
                                          "Can't allocate memory for new record instance.");
                }
                return Record::Shared(record);

            } else {
                throw IndexError("UUIDMapBlockStorage::readByUUID. "
                                     "Can't find such transactionUUID in index block.");
            }
        }

        const vector<uuids::uuid>* UUIDMapBlockStorage::keys() const {

            vector<uuids::uuid> *uuidsVector = nullptr;
            try {
                uuidsVector = new vector<uuids::uuid>;

            } catch (std::bad_alloc &e) {
                throw MemoryError("UUIDMapBlockStorage::keys. "
                                      "Can't allocate memory for new keys vector.");
            }
            uuidsVector->reserve(mIndexBlock.size());
            for (auto const &it : mIndexBlock){
                uuidsVector->push_back(it.first);
            }
            return uuidsVector;
        }

        const bool UUIDMapBlockStorage::isExist(
            const uuids::uuid &uuid) {

            return mIndexBlock.count(uuid) != 0;
        }

        void UUIDMapBlockStorage::vacuum() {

            UUIDMapBlockStorage *mapBlockStorage = nullptr;
            try {
                mapBlockStorage = new UUIDMapBlockStorage(
                    mDirectory,
                    mTempFileName);

            } catch (std::bad_alloc &e) {
                throw MemoryError("UUIDMapBlockStorage::vacuum. "
                                      "Can't allocate memory for new storage.");
            }

            for (auto const &looper : mIndexBlock){
                Record::Shared record = readByUUID(looper.first);
                mapBlockStorage->write(
                    looper.first,
                    record->data(),
                    record->bytesCount()
                );
            }

            delete mapBlockStorage;

            fclose(mFileDescriptor);
            if (remove(mFilePath.c_str()) != 0){
                throw IOError("UUIDMapBlockStorage::vacuum. "
                                  "Can't remove old *.dat file.");
            }
            if (rename(mTempFilePath.c_str(), mFilePath.c_str()) != 0){
                throw IOError("UUIDMapBlockStorage::vacuum. "
                                  "Can't rename temporary *.dat file to main *.dat file.");
            }
            obtainFileDescriptor();
        }

        void UUIDMapBlockStorage::checkDirectory() {

            if (!fs::is_directory(fs::path(mDirectory))){
                fs::create_directories(
                    fs::path(mDirectory)
                );
            }
        }

        void UUIDMapBlockStorage::removeTemporaryFile() {

            if (isFileExist(mFilePath)){
                if (isFileExist(mTempFilePath)){
                    if (remove(mTempFilePath.c_str()) != 0){
                        throw IOError("UUIDMapBlockStorage::removeTemporaryFile. "
                                          "Can't remove old *.dat file.");
                    }
                }

            } else {
                if (isFileExist(mTempFilePath)){
                    if (rename(mTempFilePath.c_str(), mFilePath.c_str()) != 0){
                        throw IOError("UUIDMapBlockStorage::removeTemporaryFile. "
                                          "Can't rename temporary *.dat file to main *.dat file.");
                    }
                }
            }
        }

        void UUIDMapBlockStorage::obtainFileDescriptor() {

            if (isFileExist(mFilePath)) {
                mFileDescriptor = fopen(
                    mFilePath.c_str(),
                    kModeUpdate.c_str()
                );
                checkFileDescriptor();

            } else {
                mFileDescriptor = fopen(
                    mFilePath.c_str(),
                    kModeCreate.c_str()
                );
                checkFileDescriptor();
                allocateFileHeader();
            }
        }

        void UUIDMapBlockStorage::checkFileDescriptor() {

            if (mFileDescriptor == NULL) {
                throw IOError("UUIDMapBlockStorage::checkFileDescriptor. "
                                  "Unable to obtain file descriptor.");
            }
            mPOSIXFileDescriptor = fileno(mFileDescriptor);
        }

        void UUIDMapBlockStorage::allocateFileHeader() {

            byte *buffer = (byte *) malloc(kFileHeaderSize);
            memset(
                buffer,
                0,
                kFileHeaderSize
            );
            fseek(
                mFileDescriptor,
                0,
                SEEK_SET
            );
            if (fwrite(buffer, 1, kFileHeaderSize, mFileDescriptor) != kFileHeaderSize) {
                if (fwrite(buffer, 1, kFileHeaderSize, mFileDescriptor) != kFileHeaderSize) {
                    free(buffer);
                    throw IOError("UUIDMapBlockStorage::allocateFileHeader. "
                                      "Can't allocate default empty file header.");
                }
            }
            free(buffer);
            syncData();
        }

        void UUIDMapBlockStorage::readFileHeader() {

            checkFileDescriptor();
            byte *headerBuffer = (byte *) malloc(kFileHeaderMapIndexOffset);
            memset(
                headerBuffer,
                0,
                kFileHeaderMapIndexOffset
            );
            fseek(
                mFileDescriptor,
                0,
                SEEK_SET
            );
            if (fread(headerBuffer, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                if (fread(headerBuffer, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                    free(headerBuffer);
                    throw IOError("UUIDMapBlockStorage::readFileHeader. "
                                      "Can't read index block offset from file header");
                }
            }
            mMapIndexOffset = *((uint32_t *) headerBuffer);
            free(headerBuffer);
            headerBuffer = (byte *) malloc(kFileHeaderMapRecordsCount);
            memset(
                headerBuffer,
                0,
                kFileHeaderMapRecordsCount
            );
            if (fread(headerBuffer, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount) {
                if (fread(headerBuffer, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount) {
                    free(headerBuffer);
                    throw IOError("UUIDMapBlockStorage::readFileHeader. "
                                      "Can't read index records count in index block from file header");
                }
            }
            mMapIndexRecordsCount = *((uint64_t *) headerBuffer);
            free(headerBuffer);
        }

        void UUIDMapBlockStorage::readIndexBlock() {

            if (mMapIndexRecordsCount > 0) {
                byte *indexBlockBuffer = (byte *) malloc(kIndexRecordSize * mMapIndexRecordsCount);
                memset(
                    indexBlockBuffer,
                    0,
                    kIndexRecordSize * mMapIndexRecordsCount
                );
                fseek(
                    mFileDescriptor,
                    mMapIndexOffset,
                    SEEK_SET
                );
                if (fread(indexBlockBuffer, 1, kIndexRecordSize * mMapIndexRecordsCount, mFileDescriptor) != kIndexRecordSize * mMapIndexRecordsCount){
                    if (fread(indexBlockBuffer, 1, kIndexRecordSize * mMapIndexRecordsCount, mFileDescriptor) != kIndexRecordSize * mMapIndexRecordsCount){
                        free(indexBlockBuffer);
                        throw IOError("UUIDMapBlockStorage::readIndexBlock. Error while reading index block from *.dat file.");
                    }
                }

                byte *indexBlockBufferOffset = indexBlockBuffer;
                for (size_t recordNumber = 0; recordNumber < mMapIndexRecordsCount; ++ recordNumber) {
                    uuids::uuid uuid;
                    memcpy(
                        &uuid,
                        indexBlockBufferOffset,
                        16
                    );
                    uint32_t *offset = new (indexBlockBufferOffset + kIndexRecordUUIDSize) uint32_t;
                    uint64_t *blockBytesCount = new (indexBlockBufferOffset + kIndexRecordUUIDSize + sizeof(uint32_t)) uint64_t;
                    mIndexBlock.insert(
                        make_pair(
                            uuid,
                            make_pair(
                                *offset,
                                *blockBytesCount)
                        )
                    );
                    indexBlockBufferOffset += kIndexRecordSize;
                }
                free(indexBlockBuffer);
            }
        }

        const long UUIDMapBlockStorage::writeData(
            const byte *block,
            const size_t blockBytesCount) {

            fseek(
                mFileDescriptor,
                0,
                SEEK_END
            );
            long offset = ftell(mFileDescriptor);
            if (fwrite(block, 1, blockBytesCount, mFileDescriptor) != blockBytesCount) {
                if (fwrite(block, 1, blockBytesCount, mFileDescriptor) != blockBytesCount) {
                    throw IOError("UUIDMapBlockStorage::writeData. "
                                      "Can't write data buffer in file.");
                }
            }
            syncData();
            return offset;
        }

        const pair<uint32_t, uint64_t> UUIDMapBlockStorage::writeIndexRecordsInMemory(
            const uuids::uuid &uuid,
            const long offset,
            const size_t blockBytesCount) {

            fseek(
                mFileDescriptor,
                0,
                SEEK_END
            );
            long offsetToIndexRecords = ftell(mFileDescriptor);
            mIndexBlock.insert(
                make_pair(
                    uuid,
                    make_pair(
                        (uint32_t) offset,
                        (uint64_t) blockBytesCount)
                )
            );
            size_t recordsCount = mIndexBlock.size();
            writeIndexBlock();
            return make_pair(
                (uint32_t) offsetToIndexRecords,
                (uint64_t) recordsCount
            );
        }

        void UUIDMapBlockStorage::writeIndexBlock() {

            for (auto const &looper : mIndexBlock){
                if (fwrite(looper.first.data, 1, kIndexRecordUUIDSize, mFileDescriptor) != kIndexRecordUUIDSize){
                    if (fwrite(looper.first.data, 1, kIndexRecordUUIDSize, mFileDescriptor) != kIndexRecordUUIDSize){
                        throw IOError("UUIDMapBlockStorage::writeIndexBlock. "
                                          "Can't transactionUUID in index block.");
                    }
                }
                if (fwrite(&looper.second.first, 1, kIndexRecordOffsetSize, mFileDescriptor) != kIndexRecordOffsetSize){
                    if (fwrite(&looper.second.first, 1, kIndexRecordOffsetSize, mFileDescriptor) != kIndexRecordOffsetSize){
                        throw IOError("UUIDMapBlockStorage::writeIndexBlock. "
                                          "Can't offset to data block in index block.");
                    }
                }
                if (fwrite(&looper.second.second, 1, kIndexRecordDataSize, mFileDescriptor) != kIndexRecordDataSize){
                    if (fwrite(&looper.second.second, 1, kIndexRecordDataSize, mFileDescriptor) != kIndexRecordDataSize){
                        throw IOError("UUIDMapBlockStorage::writeIndexBlock. "
                                          "Can't data bytes count in index block.");
                    }
                }
            }
            syncData();
        }

        void UUIDMapBlockStorage::writeFileHeader() {

            fseek(
                mFileDescriptor,
                0,
                SEEK_SET
            );
            if (fwrite(&mMapIndexOffset, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                if (fwrite(&mMapIndexOffset, 1, kFileHeaderMapIndexOffset, mFileDescriptor) != kFileHeaderMapIndexOffset) {
                    throw IOError("UUIDMapBlockStorage::writeFileHeader. "
                                      "Can't write index block offset in file header.");
                }
            }
            if (fwrite(&mMapIndexRecordsCount, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount){
                if (fwrite(&mMapIndexRecordsCount, 1, kFileHeaderMapRecordsCount, mFileDescriptor) != kFileHeaderMapRecordsCount){
                    throw IOError("UUIDMapBlockStorage::writeFileHeader. "
                                      "Can't write index records count in index block in file header.");
                }
            }
            syncData();
        }

        void UUIDMapBlockStorage::syncData() {

            fflush(mFileDescriptor);
            fsync(mPOSIXFileDescriptor);
        }

        const bool UUIDMapBlockStorage::isFileExist(
            string &fileName) {

            return fs::exists(fs::path(fileName.c_str()));
        }

        const bool UUIDMapBlockStorage::isUUIDTheIndex(
            const uuids::uuid &uuid) {

            return mIndexBlock.count(uuid) > 0;
        }
    }
}

