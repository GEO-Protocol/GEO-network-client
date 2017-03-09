#ifndef GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
#define GEO_NETWORK_CLIENT_TRUSTLINERECORD_H

#include "../base/Record.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <vector>
#include <stdint.h>

namespace db {
    namespace operations_history_storage{

        class TrustLineRecord: public Record {
        public:
            typedef shared_ptr<TrustLineRecord> Shared;

        public:
            enum TrustLineOperationType {
                Opening = 1,
                Updating,
                Closing,
            };
            typedef uint8_t SerializedTrustLineOperationType;

        public:
            TrustLineRecord(
                BytesShared buffer);

            TrustLineRecord(
                const uuids::uuid &operationUUID,
                const TrustLineRecord::TrustLineOperationType operationType,
                const NodeUUID &contractorUUID);

            TrustLineRecord(
                const uuids::uuid &operationUUID,
                const TrustLineRecord::TrustLineOperationType operationType,
                const NodeUUID &contractorUUID,
                const TrustLineAmount &amount);

            const TrustLineOperationType trustLineOperationType() const;

            const NodeUUID contractorUUID() const;

            const TrustLineAmount amount() const;

        private:
            const bool isTrustLineRecord() const;

            pair<BytesShared, size_t> serializeToBytes();

            void deserializeFromBytes(
                BytesShared buffer);

        private:
            TrustLineOperationType mTrustLineOperationType;
            NodeUUID mContractorUUID;
            TrustLineAmount mAmount;
        };

    }
}

#endif //GEO_NETWORK_CLIENT_TRUSTLINERECORD_H
