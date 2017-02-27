#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../interface/commands_interface/commands/trust_lines/SetTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/outgoing/trust_lines/SetTrustLineMessage.h"
#include "../../../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"

#include "UpdateTrustLineTransaction.h"
#include "OpenTrustLineTransaction.h"
#include "CloseTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class SetTrustLineTransaction: public TrustLineTransaction {

public:
    typedef shared_ptr<SetTrustLineTransaction> Shared;

public:
    SetTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetTrustLineCommand::Shared command,
        TrustLinesManager *manager);

    SetTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager);

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

};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINETRANSACTION_H
