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

        vector<PaymentRecord::Shared> OperationsHistoryStorage::paymentRecordsStack(
            size_t recordsCount,
            size_t fromRecord) {

            vector<PaymentRecord::Shared> result;

            try {
                vector<Record::Shared> paymentRecords = recordsStack(
                    Record::RecordType::PaymentRecordType,
                    recordsCount,
                    fromRecord);

                result.reserve(paymentRecords.size());

                for (auto &record : paymentRecords) {
                    result.push_back(static_pointer_cast<PaymentRecord>(record));
                }

                return result;

            } catch (NotFoundError) {
                return result;
            }
        }

        vector<TrustLineRecord::Shared> OperationsHistoryStorage::trustLineRecordsStack(
                size_t recordsCount,
                size_t fromRecord) {

            vector<TrustLineRecord::Shared> result;

            try {
                vector<Record::Shared> paymentRecords = recordsStack(
                    Record::RecordType::TrustLineRecordType,
                    recordsCount,
                    fromRecord);
                result.reserve(paymentRecords.size());

                for (auto &record : paymentRecords) {
                    result.push_back(static_pointer_cast<TrustLineRecord>(record));
                }

                return result;

            } catch (NotFoundError) {
                return result;
            }
        }

        vector<Record::Shared> OperationsHistoryStorage::recordsStack(
            Record::RecordType recordType,
            size_t recordsCount,
            size_t fromRecord) {

            vector<Record::Shared> records;
            records.reserve(
                recordsCount);

            int64_t reverseOffset = reverseOffsetToRequestedRecord(
                recordType,
                fromRecord);

            uint64_t findingRecordsCount = 0;
            int64_t currentCursorPosition = 0;
            bool isStartOfFile = false;

            while (!isStartOfFile) {

                fseek(
                    mFileDescriptor,
                    reverseOffset,
                    SEEK_END);

                currentCursorPosition = ftell(mFileDescriptor);

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

                    findingRecordsCount += 1;
                    records.push_back(
                        record);

                }

                if (findingRecordsCount == recordsCount) {
                    reverse(
                        records.begin(),
                        records.end());
                    return records;
                }

                reverseOffset -= Record::kRecordBytesSize;

                if (currentCursorPosition == 0) {
                    isStartOfFile = true;
                }

            }

            if (!records.empty()) {
                records.resize(
                    findingRecordsCount);
                reverse(
                    records.begin(),
                    records.end());
                return records;
            }

            throw RuntimeError("OperationsHistoryStorage::recordsStack. "
                                   "Can't complete operation.");
        }

        int64_t OperationsHistoryStorage::reverseOffsetToRequestedRecord(
            Record::RecordType recordType,
            size_t fromRecord) {

            int64_t reverseOffset = 0;
            size_t typesMatchingCounter = 0;
            bool isStartOfFile = false;

            fseek(
                mFileDescriptor,
                0,
                SEEK_END);

            while (!isStartOfFile) {

                reverseOffset -= Record::kRecordBytesSize;

                fseek(
                    mFileDescriptor,
                    reverseOffset,
                    SEEK_END);

                if (ftell(mFileDescriptor) == 0) {
                    isStartOfFile = true;
                }

                BytesShared recordBytesBuffer = tryCalloc(
                    Record::kRecordBytesSize);

                if (fread(recordBytesBuffer.get(), 1, Record::kRecordBytesSize, mFileDescriptor) != Record::kRecordBytesSize){
                    if (fread(recordBytesBuffer.get(), 1, Record::kRecordBytesSize, mFileDescriptor) != Record::kRecordBytesSize){
                        throw IOError("OperationsHistoryStorage::reverseOffsetToRequestedRecord. "
                                          "Can't read next record from file.");
                    }
                }

                Record::SerializedRecordType *fetchedRecordType = new (recordBytesBuffer.get()) Record::SerializedRecordType;
                if (recordType == (Record::RecordType) *fetchedRecordType) {

                    if (fromRecord == typesMatchingCounter) {
                        return reverseOffset;
                    }

                    typesMatchingCounter += 1;

                }

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