#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/RejectTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class RejectTrustLineTransaction : public TrustLineTransaction {
public:
    typedef shared_ptr<RejectTrustLineTransaction> Shared;

public:
    RejectTrustLineTransaction(
        const NodeUUID &nodeUUID,
        RejectTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    RejectTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger *logger);

    RejectTrustLineMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
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
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINETRANSACTION_H
