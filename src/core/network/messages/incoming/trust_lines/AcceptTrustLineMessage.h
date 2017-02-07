#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.hpp"
#include "../../result/MessageResult.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <memory>
#include <utility>
#include <cstdlib>
#include <stdint.h>

using namespace std;

class AcceptTrustLineMessage : public TrustLinesMessage {
public:
    typedef shared_ptr<AcceptTrustLineMessage> Shared;

public:
    AcceptTrustLineMessage(
        BytesShared buffer);

    const MessageType typeID() const;

    const TrustLineAmount &amount() const;

    pair<BytesShared, size_t> serializeToBytes();

    static const size_t kRequestedBufferSize();

    MessageResult::SharedConst resultAccepted() const;

    MessageResult::SharedConst resultConflict() const;

    MessageResult::SharedConst resultTransactionConflict() const;

    MessageResult::SharedConst customCodeResult(
        uint16_t code) const;

private:
    void deserializeFromBytes(
        BytesShared buffer);

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeConflict = 409;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    TrustLineAmount mTrustLineAmount;
};
#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
