#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "../../base/trust_lines/TrustLinesMessage.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include "../../result/MessageResult.h"

#include <memory>
#include <utility>
#include <stdint.h>

using namespace std;

class AcceptTrustLineMessage : public TrustLinesMessage {
public:
    typedef shared_ptr<AcceptTrustLineMessage> Shared;

public:
    AcceptTrustLineMessage(
        BytesShared buffer);

    const TrustLineAmount &amount() const;

    static const size_t kRequestedBufferSize();

    pair<BytesShared, size_t> serializeToBytes();

    MessageResult::SharedConst resultAccepted() const;

    MessageResult::SharedConst resultConflict() const;

    MessageResult::SharedConst resultTransactionConflict() const;

private:
    const MessageType typeID() const;

    void deserializeFromBytes(
        BytesShared buffer);

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeConflict = 409;
// There are more than one transaction with same type that are processed at one moment
    static const uint16_t kResultCodeTransactionConflict = 429;

private:
    TrustLineAmount mTrustLineAmount;
};
#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
