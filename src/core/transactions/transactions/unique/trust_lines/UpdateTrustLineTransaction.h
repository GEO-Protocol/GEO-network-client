#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../../../db/operations_history_storage/record/trust_line/TrustLineRecord.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/incoming/trust_lines/UpdateTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

using namespace db::operations_history_storage;

class UpdateTrustLineTransaction : public TrustLineTransaction {
public:
    typedef shared_ptr<UpdateTrustLineTransaction> Shared;

private:
    enum Stages{
        CheckJournal = 1,
        CheckUnicity,
        CheckIncomingDirection
    };

public:
    UpdateTrustLineTransaction(
        const NodeUUID &nodeUUID,
        UpdateTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    UpdateTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    UpdateTrustLineMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool checkJournal();

    bool isTransactionToContractorUnique();

    bool isIncomingTrustLineDirectionExisting();

    bool isIncomingTrustLineCouldBeModified();

    void updateIncomingTrustAmount();

    void logUpdatingTrustLineOperation();

    void sendResponseCodeToContractor(
        const uint16_t code);

private:
    UpdateTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    OperationsHistoryStorage *mOperationsHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
