#include "OperationsHistoryStorage.h"

namespace db {
    namespace operations_history_storage {

        OperationsHistoryStorage::OperationsHistoryStorage(
            const string &directory,
            const string &fileName):

            mDirectory(directory),
            mFileName(fileName),
            mFilePath(mDirectory + "/" + mFileName) {

            checkDirectory();
            obtainFileDescriptor();
        }

        OperationsHistoryStorage::~OperationsHistoryStorage() {

            fclose(mFileDescriptor);
        }

        void OperationsHistoryStorage::addRecord(
            Record::Shared record) {

            auto bytesAndCount = record->serializeToBytes();

            fseek(
                mFileDescriptor,
                0,
                SEEK_END);

            if (fwrite(bytesAndCount.first.get(), 1, bytesAndCount.second, mFileDescriptor) != bytesAndCount.second) {
                if (fwrite(bytesAndCount.first.get(), 1, bytesAndCount.second, mFileDescriptor) != bytesAndCount.second) {
                    throw IOError("OperationsHistoryStorage::addRecord. "
                                      "Can't add record in file.");
                }
            }

            syncData();
        }

        vector<Record::Shared> OperationsHistoryStorage::recordsStack(
            Record::RecordType recordType,
            size_t recordsCount,
            size_t fromRecord) {

            vector<Record::Shared> records;

            int64_t reverseOffset = reverseOffsetToRequestedRecord(
                recordType,
                fromRecord);

            while (ftell(mFileDescriptor) != 0) {

                fseek(
                    mFileDescriptor,
                    reverseOffset,
                    SEEK_END);

                BytesShared recordBytesBuffer = tryMalloc(
                    Record::kRecordBytesSize);

                if (fread(recordBytesBuffer.get(), 1, Record::kRecordBytesSize, mFileDescriptor) != Record::kRecordBytesSize){
                    if (fread(recordBytesBuffer.get(), 1, Record::kRecordBytesSize, mFileDescriptor) != Record::kRecordBytesSize){
                        throw IOError("OperationsHistoryStorage::recordsStack. "
                                          "Can't read next record from file.");
                    }
                }

                Record::SerializedRecordType *fetchedRecordType = new (recordBytesBuffer.get()) Record::SerializedRecordType;
                if (recordType == (Record::RecordType) *fetchedRecordType) {

                    Record::Shared record;

                    if (recordType == Record::RecordType::TrustLineRecordType) {

                        try {
                            record = make_shared<TrustLineRecord>(
                                recordBytesBuffer);

                        } catch (exception&) {
                            throw RuntimeError("OperationsHistoryStorage::recordsStack. "
                                                   "Can't create record from bytes.");
                        }

                    } else if (recordType == Record::RecordType::PaymentRecordType) {

                        try {
                            record = make_shared<PaymentRecord>(
                                recordBytesBuffer);

                        } catch (exception&) {
                            throw RuntimeError("OperationsHistoryStorage::recordsStack. "
                                                   "Can't create record from bytes.");
                        }

                    } else {
                        throw ConflictError("OperationsHistoryStorage::recordsStack. "
                                                "Unexpected record type.");
                    }

                    records.push_back(
                        record);

                }

                reverseOffset -= Record::kRecordBytesSize;

                if (records.size() == recordsCount) {
                    break;
                }


            }

            return records;
        }

        int64_t OperationsHistoryStorage::reverseOffsetToRequestedRecord(
            Record::RecordType recordType,
            size_t fromRecord) {

            int64_t reverseOffset = 0;
            size_t typesMatchingCounter = 0;

            while (ftell(mFileDescriptor) != 0) {

                reverseOffset -= Record::kRecordBytesSize;

                fseek(
                    mFileDescriptor,
                    reverseOffset,
                    SEEK_END);

                BytesShared recordBytesBuffer = tryCalloc(
                    Record::kRecordBytesSize);

                if (fread(recordBytesBuffer.get(), 1, Record::kRecordBytesSize, mFileDescriptor) != Record::kRecordBytesSize){
                    if (fread(recordBytesBuffer.get(), 1, Record::kRecordBytesSize, mFileDescriptor) != Record::kRecordBytesSize){
                        throw IOError("OperationsHistoryStorage::recordsStack. "
                                          "Can't read next record from file.");
                    }
                }

                Record::SerializedRecordType *fetchedRecordType = new (recordBytesBuffer.get()) Record::SerializedRecordType;
                if (recordType == (Record::RecordType) *fetchedRecordType) {

                    if (fromRecord == typesMatchingCounter) {
                        return reverseOffset;
                    }

                }

                typesMatchingCounter += 1;

            }

            throw NotFoundError("OperationsHistoryStorage::reverseOffsetToRequestedRecord: "
                                    "There are no records of requested type.");
        }

        void OperationsHistoryStorage::checkDirectory() {

            if (!fs::is_directory(fs::path(mDirectory))){
                fs::create_directories(
                    fs::path(mDirectory));
            }
        }

        void OperationsHistoryStorage::obtainFileDescriptor() {

            if (isFileExist(mFilePath)) {
                mFileDescriptor = fopen(
                    mFilePath.c_str(),
                    kModeUpdate.c_str());

            } else {
                mFileDescriptor = fopen(
                    mFilePath.c_str(),
                    kModeCreate.c_str());
            }
            checkFileDescriptor();
        }

        const bool OperationsHistoryStorage::isFileExist(
            string &fileName) const {

            return fs::exists(
                fs::path(fileName.c_str()));
        }

        void OperationsHistoryStorage::checkFileDescriptor() {

            if (mFileDescriptor == NULL) {
                throw IOError("OperationsHistoryStorage::checkFileDescriptor. "
                                  "Unable to obtain file descriptor.");
            }
            mPOSIXFileDescriptor = fileno(mFileDescriptor);
        }

        void OperationsHistoryStorage::syncData() {

            fflush(mFileDescriptor);
            fdatasync(mPOSIXFileDescriptor);
        }

    }
}