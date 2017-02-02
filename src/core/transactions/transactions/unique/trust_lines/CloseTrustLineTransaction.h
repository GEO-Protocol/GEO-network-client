#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/outgoing/trust_lines/CloseTrustLineMessage.h"
#include "../../../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "RejectTrustLineTransaction.h"
#include "OpenTrustLineTransaction.h"
#include "SetTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

class CloseTrustLineTransaction : public UniqueTransaction {

public:
    typedef shared_ptr<CloseTrustLineTransaction> Shared;

public:
    CloseTrustLineTransaction(
        NodeUUID &nodeUUID,
        CloseTrustLineCommand::Shared command,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    CloseTrustLineTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    CloseTrustLineCommand::Shared command() const;

    TransactionResult::Shared run();

private:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

    bool checkSameTypeTransactions();

    bool checkTrustLineDirectionExisting();

    bool checkDebt();

    void suspendTrustLineToContractor();

    void closeTrustLine();

    TransactionResult::Shared checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::Shared waitingForResponseState();

    TransactionResult::Shared resultOk();

    TransactionResult::Shared trustLineAbsentResult();

    TransactionResult::Shared conflictErrorResult();

    TransactionResult::Shared noResponseResult();

    TransactionResult::Shared transactionConflictResult();

    TransactionResult::Shared unexpectedErrorResult();

private:
    const uint64_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    CloseTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;

};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
