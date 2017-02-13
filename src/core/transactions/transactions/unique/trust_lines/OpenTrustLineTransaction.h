#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

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

#include <memory>
#include <utility>
#include <cstdint>

class OpenTrustLineTransaction: public TrustLineTransaction {

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

    void openTrustLine();

    TransactionResult::SharedConst resultOk();

    TransactionResult::SharedConst trustLinePresentResult();

    TransactionResult::SharedConst conflictErrorResult();

    TransactionResult::SharedConst noResponseResult();

    TransactionResult::SharedConst transactionConflictResult();

    TransactionResult::SharedConst unexpectedErrorResult();

private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    OpenTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    Logger *mlogger;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
