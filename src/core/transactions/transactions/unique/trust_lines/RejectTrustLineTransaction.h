#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../db/operations_history_storage/storage/OperationsHistoryStorage.h"
#include "../../../../db/operations_history_storage/record/trust_line/TrustLineRecord.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/incoming/trust_lines/RejectTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

using namespace db::operations_history_storage;

class RejectTrustLineTransaction : public TrustLineTransaction {
public:
    typedef shared_ptr<RejectTrustLineTransaction> Shared;

public:
    RejectTrustLineTransaction(
        const NodeUUID &nodeUUID,
        RejectTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    RejectTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        OperationsHistoryStorage *historyStorage);

    RejectTrustLineMessage::Shared message() const;

    pair<BytesShared, size_t> serializeToBytes() const;

    TransactionResult::SharedConst run();

private:
    void deserializeFromBytes(
        BytesShared buffer);

    bool isTransactionToContractorUnique();

    bool isIncomingTrustLineDirectionExisting();

    bool checkDebt();

    void suspendTrustLineDirectionFromContractor();

    void rejectTrustLine();

    void logRejectingTrustLineOperation();

    void sendResponseCodeToContractor(
        const uint16_t code);

private:
    RejectTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    OperationsHistoryStorage *mOperationsHistoryStorage;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
