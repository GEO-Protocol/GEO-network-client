#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/time/TimeUtils.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../interface/commands_interface/commands/trust_lines/OpenTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/OpenTrustLineMessage.h"
#include "../../../../network/messages/trust_lines/AcceptTrustLineMessage.h"

#include "AcceptTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"
#include "../../../../common/exceptions/RuntimeError.h"

#include "../../../../transactions/transactions/routing_tables/TrustLineStatesHandlerTransaction.h"

#include <memory>
#include <utility>
#include <cstdint>

class OpenTrustLineTransaction: public TrustLineTransaction {
public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

private:
    enum Stages {
        CheckUnicity = 1,
        CheckOutgoingDirection,
        CheckContext
    };

public:
    OpenTrustLineTransaction(
        const NodeUUID &nodeUUID,
        OpenTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    OpenTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    OpenTrustLineCommand::Shared command() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isOutgoingTrustLineDirectionExisting();

    TransactionResult::SharedConst checkTransactionContext();

    void sendMessageToRemoteNode();

    TransactionResult::SharedConst waitingForResponseState();

    void openTrustLine();

    void logOpeningTrustLineOperation();

    TransactionResult::SharedConst resultOk();

    TransactionResult::SharedConst resultTrustLineIsAlreadyPresent();

    TransactionResult::SharedConst resultConflictWithOtherOperation();

    TransactionResult::SharedConst resultRemoteNodeIsInaccessible();

    TransactionResult::SharedConst resultProtocolError();

private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    OpenTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};

#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
