/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_REJECTOUTGOINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_REJECTOUTGOINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"

#include "../../../network/messages/base/transaction/ConfirmationMessage.h"

class RejectOutgoingTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<RejectOutgoingTrustLineTransaction> Shared;

public:
    RejectOutgoingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        ConfirmationMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const
    noexcept;

protected:
    ConfirmationMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_REJECTOUTGOINGTRUSTLINETRANSACTION_H
