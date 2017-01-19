#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../network/messages/incoming/AcceptTrustLineMessage.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../scheduler/TransactionsScheduler.h"

#include "../../../manager/TransactionsManager.h"

class AcceptTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

public:
    AcceptTrustLineTransaction(
        NodeUUID &nodeUUID,
        AcceptTrustLineMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    AcceptTrustLineMessage::Shared message() const;

    TransactionResult::Shared run();

private:
    bool checkJournal();

    bool checkSameTypeTransactions();

    bool checkTrustLineDirection();

    bool checkTrustLineAmount();

    void createTrustLine();

    void sendResponse(
        uint16_t code);

    TransactionResult::Shared makeResult(
        MessageResult::Shared messageResult);


private:
    AcceptTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
