#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../common/Types.h"
#include "../../../common/memory/MemoryUtils.h"

#include "../../../io/storage/StorageHandler.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"

#include "../../../network/messages/Message.hpp"
#include "../../../network/messages/trust_lines/UpdateTrustLineMessage.h"
#include "../../../network/messages/response/Response.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>
#include <cstdint>


class UpdateTrustLineTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<UpdateTrustLineTransaction> Shared;

public:
    UpdateTrustLineTransaction(
        const NodeUUID &nodeUUID,
        UpdateTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const
        noexcept;

    void updateHistory(
        IOTransaction::Shared ioTransaction);

protected:
    UpdateTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINETRANSACTION_H
