#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../network/messages/incoming/RejectTrustLineMessage.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include "../../../manager/TransactionsManager.h"

class RejectTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<RejectTrustLineTransaction> Shared;

public:
    RejectTrustLineTransaction(
        NodeUUID &nodeUUID,
        RejectTrustLineMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    RejectTrustLineMessage::Shared message() const;

    TransactionResult::Shared run();

private:
    bool checkSameTypeTransactions();

    bool checkTrustLineExisting();

    bool checkDebt();

    void suspendTrustLineFromContractor();

    void closeTrustLine();

    void sendResponse(
        uint16_t code);

    TransactionResult::Shared makeResult(
        MessageResult::Shared messageResult);

private:
    RejectTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
