#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../../../db/operations_history_storage/record/trust_line/TrustLineRecord.h"

#include "../../../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/CloseTrustLineMessage.h"
#include "../../../../network/messages/trust_lines/RejectTrustLineMessage.h"

#include "RejectTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

using namespace db::operations_history_storage;

class CloseTrustLineTransaction: public TrustLineTransaction {
public:
    typedef shared_ptr<CloseTrustLineTransaction> Shared;

private:
    enum Stages {
        CheckUnicity = 1,
        CheckOutgoingDirection,
        CheckDebt,
        CheckContext
    };

public:
    CloseTrustLineTransaction(
        const NodeUUID &nodeUUID,
        CloseTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    CloseTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

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

    void logClosingTrustLineOperation();

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
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    CloseTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    OperationsHistoryStorage *mOperationsHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
