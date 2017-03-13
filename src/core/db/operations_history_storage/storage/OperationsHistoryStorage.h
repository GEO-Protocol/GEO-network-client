#ifndef GEO_NETWORK_CLIENT_OPERATIONSHISTORYSTORAGE_H
#define GEO_NETWORK_CLIENT_OPERATIONSHISTORYSTORAGE_H

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../record/base/Record.h"
#include "../record/trust_line/TrustLineRecord.h"
#include "../record/payment/PaymentRecord.h"

#include "../../../common/exceptions/IOError.h"
#include "../../../common/exceptions/ConflictError.h"
#include "../../../common/exceptions/RuntimeError.h"
#include "../../../common/exceptions/NotFoundError.h"

#include <boost/filesystem.hpp>

#include <vector>
#include <stdio.h>
#include <stdint.h>

namespace db {
    namespace operations_history_storage {

        namespace fs = boost::filesystem;
        namespace uuids = boost::uuids;

        using namespace std;

        class OperationsHistoryStorage {

        public:
            OperationsHistoryStorage(
                const string &directory,
                const string &fileName);

            ~OperationsHistoryStorage();

            void addRecord(
                Record::Shared record);

            template<class T>
            vector<shared_ptr<T>> recordsStack(
                Record::RecordType recordType,
                size_t recordsCount,
                size_t fromRecord);

        private:
            int64_t reverseOffsetToRequestedRecord(
                Record::RecordType recordType,
                size_t fromRecord);

            void checkDirectory();

            void obtainFileDescriptor();

            void checkFileDescriptor();

            const bool isFileExist(
                string &fileName) const;

            void syncData();

        private:
            const string kModeCreate = "w+";
            const string kModeUpdate = "r+";

        private:
            string mDirectory;
            string mFileName;
            string mFilePath;

            FILE *mFileDescriptor;
            int mPOSIXFileDescriptor;
        };

    }
}

#endif //GEO_NETWORK_CLIENT_OPERATIONSHISTORYSTORAGE_H
