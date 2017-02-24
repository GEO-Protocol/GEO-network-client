#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "AcceptTrustLineTransaction.h"
#include "UpdateTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class RejectTrustLineTransaction : public TrustLineTransaction {

public:
    typedef shared_ptr<RejectTrustLineTransaction> Shared;

public:
    RejectTrustLineTransaction(
        const NodeUUID &nodeUUID,
        RejectTrustLineMessage::Shared message,
        TrustLinesManager *manager);

    RejectTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

    RejectTrustLineMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isIncomingTrustLineDirectionExisting();

    bool checkDebt();

    void suspendTrustLineDirectionFromContractor();

    void rejectTrustLine();

    void sendResponseCodeToContractor(
        uint16_t code);

private:
    RejectTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
