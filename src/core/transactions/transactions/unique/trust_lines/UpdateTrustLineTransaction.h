#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "AcceptTrustLineTransaction.h"
#include "RejectTrustLineTransaction.h"

#include "../../../manager/TransactionsManager.h"

class UpdateTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<UpdateTrustLineTransaction> Shared;

public:
    UpdateTrustLineTransaction(
        NodeUUID &nodeUUID,
        UpdateTrustLineMessage::Shared message,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    UpdateTrustLineTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    UpdateTrustLineMessage::Shared message() const;

    TransactionResult::Shared run();

private:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

    bool checkJournal();

    bool checkSameTypeTransactions();

    bool checkTrustLineDirectionExisting();

    bool checkTrustLineAmount();

    void updateIncomingTrustAmount();

    void sendResponse(
        uint16_t code);

    TransactionResult::Shared makeResult(
        MessageResult::Shared messageResult);

private:
    UpdateTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
