#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H

#include "TrustLineTransaction.h"

#include "../../../../common/Types.h"
#include "../../../../common/memory/MemoryUtils.h"

#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../../network/messages/Message.hpp"
#include "../../../../network/messages/trust_lines/UpdateTrustLineMessage.h"
#include "../../../../network/messages/response/Response.h"

#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>

class UpdateTrustLineTransaction : public TrustLineTransaction {
public:
    typedef shared_ptr<UpdateTrustLineTransaction> Shared;

public:
    UpdateTrustLineTransaction(
        const NodeUUID &nodeUUID,
        UpdateTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger);

    UpdateTrustLineTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger);

    UpdateTrustLineMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    bool isIncomingTrustLineDirectionExisting();

    bool isAmountValid(const TrustLineAmount &amount);

    void updateIncomingTrustAmount();

    void logUpdatingTrustLineOperation();

private:
    UpdateTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
