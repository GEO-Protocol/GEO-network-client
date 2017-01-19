#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../interface/commands/commands/trust_lines/OpenTrustLineCommand.h"

#include "AcceptTrustLineTransaction.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/outgoing/OpenTrustLineMessage.h"
#include "../../../../network/messages/incoming/AcceptTrustLineMessage.h"

#include "../../../manager/TransactionsManager.h"

class OpenTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

public:
    OpenTrustLineTransaction(
        NodeUUID &nodeUUID,
        OpenTrustLineCommand::Shared command,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    OpenTrustLineCommand::Shared command() const;

    TransactionResult::Shared run();

private:
    bool checkSameTypeTransactions();

    bool checkTrustLineDirection();

    TransactionResult::Shared checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::Shared waitingForResponseState();

    void createTrustLine();

    TransactionResult::Shared resultOk();

    TransactionResult::Shared trustLinePresentResult();

    TransactionResult::Shared conflictErrorResult();

    TransactionResult::Shared noResponseResult();

    TransactionResult::Shared transactionConflictResult();

    TransactionResult::Shared unexpectedErrorResult();

private:
    const uint64_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    OpenTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
