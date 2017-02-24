#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/incoming/trust_lines/AcceptTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "RejectTrustLineTransaction.h"
#include "UpdateTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class AcceptTrustLineTransaction : public TrustLineTransaction {

public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

public:
    AcceptTrustLineTransaction(
        const NodeUUID &nodeUUID,
        AcceptTrustLineMessage::Shared message,
        TrustLinesManager *manager);

    AcceptTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

    AcceptTrustLineMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool checkJournal();

    bool isTransactionToContractorUnique();

    bool isIncomingTrustLineDirectionExisting();

    bool isIncomingTrustLineAlreadyAccepted();

    void acceptTrustLine();

    void sendResponseCodeToContractor(
        uint16_t code);

private:
    AcceptTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
