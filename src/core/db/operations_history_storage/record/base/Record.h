#ifndef GEO_NETWORK_CLIENT_RECORD_H
#define GEO_NETWORK_CLIENT_RECORD_H

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

namespace db{
    namespace operations_history_storage{

        namespace uuids = boost::uuids;

        class Record {
        public:
            typedef shared_ptr<db::operations_history_storage::Record> Shared;

        public:
            enum RecordType {
                TrustLineRecordType = 1,
                PaymentRecordType
            };
            typedef uint8_t SerializedRecordType;

        public:
            virtual const bool isTrustLineRecord() const;

            virtual const bool isPaymentRecord() const;

            const Record::RecordType recordType() const;

            const uuids::uuid operationUUID() const;

            const DateTime operationTimestamp() const;

            virtual pair<BytesShared, size_t> serializeToBytes();

        protected:
            Record();

            Record(
                const Record::RecordType recordType,
                const uuids::uuid &operationUUID);

            virtual void deserializeFromBytes(
                BytesShared buffer);

            static const size_t kOffsetToInheritedBytes();

        public:
            static const size_t kRecordBytesSize = 128;

        protected:
            static const size_t kOperationUUIDBytesSize = 16;

        private:
            RecordType mRecordType;
            uuids::uuid mOperationUUID;
            DateTime mOperationTimestamp;
        };
    }
}

#endif //GEO_NETWORK_CLIENT_RECORD_H
