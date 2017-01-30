#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"

#include "UpdateTrustLineTransaction.h"
#include "OpenTrustLineTransaction.h"
#include "CloseTrustLineTransaction.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/outgoing/SetTrustLineMessage.h"
#include "../../../../network/messages/incoming/UpdateTrustLineMessage.h"

#include "../../../manager/TransactionsManager.h"

#include "../../../../common/exceptions/ConflictError.h"

class SetTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<SetTrustLineTransaction> Shared;

public:
    SetTrustLineTransaction(
        NodeUUID &nodeUUID,
        SetTrustLineCommand::Shared command,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    SetTrustLineTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    SetTrustLineCommand::Shared command() const;

    TransactionResult::Shared run();

private:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

    bool checkSameTypeTransactions();

    bool checkTrustLineDirectionExisting();

    TransactionResult::Shared checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::Shared waitingForResponseState();

    void setOutgoingTrustAmount();

    TransactionResult::Shared resultOk();

    TransactionResult::Shared trustLineAbsentResult();

    TransactionResult::Shared conflictErrorResult();

    TransactionResult::Shared noResponseResult();

    TransactionResult::Shared transactionConflictResult();

    TransactionResult::Shared unexpectedErrorResult();

private:
    const uint64_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    SetTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;

};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
