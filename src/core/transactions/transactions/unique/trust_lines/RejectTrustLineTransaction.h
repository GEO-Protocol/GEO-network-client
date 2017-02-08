#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "AcceptTrustLineTransaction.h"
#include "UpdateTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"


class RejectTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<RejectTrustLineTransaction> Shared;

public:
    RejectTrustLineTransaction(
        NodeUUID &nodeUUID,
        RejectTrustLineMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    RejectTrustLineTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    RejectTrustLineMessage::Shared message() const;

    TransactionResult::Shared run();

private:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

    bool checkSameTypeTransactions();

    bool checkTrustLineDirectionExisting();

    bool checkDebt();

    void suspendTrustLineFromContractor();

    void rejectTrustLine();

    void sendResponse(
        uint16_t code);

    TransactionResult::Shared makeResult(
        MessageResult::SharedConst messageResult);

private:
    RejectTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
