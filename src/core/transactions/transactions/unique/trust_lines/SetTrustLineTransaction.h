#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../../../db/operations_history_storage/record/trust_line/TrustLineRecord.h"

#include "../../../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/outgoing/trust_lines/SetTrustLineMessage.h"
#include "../../../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"

#include "UpdateTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

using namespace db::operations_history_storage;

class SetTrustLineTransaction: public TrustLineTransaction {
public:
    typedef shared_ptr<SetTrustLineTransaction> Shared;

private:
    enum Stages {
        CheckUnicity = 1,
        CheckOutgoingDirection,
        CheckContext
    };

public:
    SetTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    SetTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    SetTrustLineCommand::Shared command() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isOutgoingTrustLineDirectionExisting();

    TransactionResult::SharedConst checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::SharedConst waitingForResponseState();

    void setOutgoingTrustAmount();

    void logSetTrustLineOperation();

    TransactionResult::SharedConst resultOk();

    TransactionResult::SharedConst trustLineAbsentResult();

    TransactionResult::SharedConst conflictErrorResult();

    TransactionResult::SharedConst noResponseResult();

    TransactionResult::SharedConst transactionConflictResult();

    TransactionResult::SharedConst unexpectedErrorResult();

private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    SetTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    OperationsHistoryStorage *mOperationsHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
