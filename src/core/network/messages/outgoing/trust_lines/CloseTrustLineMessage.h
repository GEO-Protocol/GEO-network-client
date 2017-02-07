#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.hpp"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

class CloseTrustLineMessage: public TrustLinesMessage {

public:
    CloseTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        NodeUUID &contractorUUID
    );

    const MessageType typeID() const;

    pair<BytesShared, size_t> serializeToBytes();

private:
    void deserializeFromBytes(
        BytesShared buffer);

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
