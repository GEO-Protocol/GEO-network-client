#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/outgoing/trust_lines/CloseTrustLineMessage.h"
#include "../../../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"

#include "../../../scheduler/TransactionsScheduler.h"
#include "RejectTrustLineTransaction.h"
#include "OpenTrustLineTransaction.h"
#include "SetTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class CloseTrustLineTransaction: public TrustLineTransaction {

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

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isOutgoingTrustLineDirectionExisting();

    bool checkDebt();

    void suspendTrustLineDirectionToContractor();

    void closeTrustLine();

    TransactionResult::SharedConst checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::SharedConst waitingForResponseState();

    TransactionResult::SharedConst resultOk();

    TransactionResult::SharedConst trustLineAbsentResult();

    TransactionResult::SharedConst conflictErrorResult();

    TransactionResult::SharedConst noResponseResult();

    TransactionResult::SharedConst transactionConflictResult();

    TransactionResult::SharedConst unexpectedErrorResult();

private:
    const uint64_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    CloseTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;

};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
