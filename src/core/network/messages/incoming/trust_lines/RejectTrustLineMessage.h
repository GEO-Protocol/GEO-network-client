#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H

#include "../../base/trust_lines/TrustLinesMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/NodeUUID.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../result/MessageResult.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class RejectTrustLineMessage : public TrustLinesMessage {
public:
    typedef shared_ptr<RejectTrustLineMessage> Shared;

public:
    RejectTrustLineMessage(
        BytesShared buffer);

    const NodeUUID &contractorUUID() const;

    const size_t kRequestedBufferSize();

    pair<BytesShared, size_t> serializeToBytes();

    MessageResult::SharedConst resultRejected();

    MessageResult::SharedConst resultRejectDelayed();

    MessageResult::SharedConst resultTransactionConflict() const;

private:
    const MessageType typeID() const;

    void deserializeFromBytes(
        BytesShared buffer);

public:
    static const uint16_t kResultCodeRejected = 200;
    static const uint16_t kResultCodeRejectDelayed = 202;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
