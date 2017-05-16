#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../interface/commands_interface/commands/trust_lines/CloseTrustLineCommand.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/CloseTrustLineMessage.h"
#include "../../../../network/messages/trust_lines/RejectTrustLineMessage.h"

#include "RejectTrustLineTransaction.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include "../../../../transactions/transactions/routing_tables/TrustLineStatesHandlerTransaction.h"

#include <memory>
#include <utility>
#include <cstdint>

class CloseTrustLineTransaction: public TrustLineTransaction {
public:
    typedef shared_ptr<CloseTrustLineTransaction> Shared;

private:
    enum Stages {
        CheckContractorUUIDValidity = 1,
        CheckUnicity,
        SendNotifyMessageToContractor,
        CheckOutgoingDirection,
        CheckDebt,
        CheckContext
    };

public:
    CloseTrustLineTransaction(
        const NodeUUID &nodeUUID,
        CloseTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    CloseTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    CloseTrustLineCommand::Shared command() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isOutgoingTrustLineDirectionExisting();

    bool trustLineIsAvailableForDelete();

    bool checkDebt();

    void closeTrustLine();

    void logClosingTrustLineOperation();

    void sendMessageToRemoteNode();

    TransactionResult::SharedConst waitingForResponseState();

    TransactionResult::SharedConst resultOk(uint16_t code);

    TransactionResult::SharedConst resultTrustLineAbsent();

    TransactionResult::SharedConst resultConflictWithOtherOperation();

    TransactionResult::SharedConst resultRemoteNodeIsInaccessible();

    TransactionResult::SharedConst resultProtocolError();

private:
    const uint16_t kConnectionTimeout = 2000;
    const uint16_t kMaxRequestsCount = 5;

    CloseTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINETRANSACTION_H
