#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "../../base/UniqueTransaction.h"

#include "../../../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/outgoing/trust_lines/OpenTrustLineMessage.h"
#include "../../../../network/messages/incoming/trust_lines/AcceptTrustLineMessage.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "AcceptTrustLineTransaction.h"
#include "SetTrustLineTransaction.h"
#include "CloseTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

class OpenTrustLineTransaction: public UniqueTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

public:
    OpenTrustLineTransaction(
        NodeUUID &nodeUUID,
        OpenTrustLineCommand::Shared command,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    OpenTrustLineTransaction(
        BytesShared buffer,
        TransactionsScheduler *scheduler,
        TrustLinesManager *manager);

    OpenTrustLineCommand::Shared command() const;

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

    void openTrustLine();

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
