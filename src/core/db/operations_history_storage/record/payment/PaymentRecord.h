#ifndef GEO_NETWORK_CLIENT_PAYMENTRECORD_H
#define GEO_NETWORK_CLIENT_PAYMENTRECORD_H

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
    namespace operations_history_storage {

        class PaymentRecord: public Record {
        public:
            typedef shared_ptr<PaymentRecord> Shared;

        public:
            enum PaymentOperationType {
                OutgoingPaymentType = 1,
                IncomingPaymentType
            };
            typedef uint8_t SerializedPaymentOperationType;

        public:
            PaymentRecord(
                BytesShared buffer);

            PaymentRecord(
                const uuids::uuid &operationUUID,
                const PaymentRecord::PaymentOperationType operationType,
                const NodeUUID &contractorUUID,
                const TrustLineAmount &amount);

            const PaymentOperationType paymentOperationType() const;

            const NodeUUID contractorUUID() const;

            const TrustLineAmount amount() const;

        private:
            const bool isPaymentRecord() const;

            pair<BytesShared, size_t> serializeToBytes();

            void deserializeFromBytes(
                BytesShared buffer);

        private:
            PaymentOperationType mPaymentOperationType;
            NodeUUID mContractorUUID;
            TrustLineAmount mAmount;
        };

    }
}

#endif //GEO_NETWORK_CLIENT_PAYMENTRECORD_H
