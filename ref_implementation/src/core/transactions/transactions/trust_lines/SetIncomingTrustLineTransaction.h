/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationCacheManager.h"
#include "../../../max_flow_calculation/cashe/MaxFlowCalculationNodeCacheManager.h"
#include "../../../io/storage/record/trust_line/TrustLineRecord.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineMessage.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineFromGatewayMessage.h"
#include "../../../network/messages/gateway_notification/GatewayNotificationMessage.h"
#include "../../../network/messages/base/transaction/ConfirmationMessage.h"


/**
 * This transaction is used to create/update/close the incoming trust line from the remote contractor.
 *
 * In case if it would be launched against new one contractor with non zero amount -
 * then new one incoming trust line would be created.
 *
 * In case if it would be launched against already present contractor, and transaction amount would not be zero -
 * then incoming trust line to this contractor would be updated to the new amount.
 *
 * In case if it would be launched against already present contractor, and transaction amount WOULD BE zero -
 * then incoming trust line to this contractor would be closed.
 */
class SetIncomingTrustLineTransaction:
    public BaseTransaction {

public:
    typedef shared_ptr<SetIncomingTrustLineTransaction> Shared;

public:
    SetIncomingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        bool iAmGateway,
        Logger &logger)
        noexcept;

    SetIncomingTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineFromGatewayMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
        MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
        bool iAmGateway,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

protected:
    TransactionResult::SharedConst resultDone();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

protected: // log
    const string logHeader() const
        noexcept;

protected:
    SetIncomingTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    MaxFlowCalculationCacheManager *mMaxFlowCalculationCacheManager;
    MaxFlowCalculationNodeCacheManager *mMaxFlowCalculationNodeCacheManager;
    bool mIAmGateway;
    bool mSenderIsGateway;
};


#endif //GEO_NETWORK_CLIENT_SETINCOMINGTRUSTLINETRANSACTION_H
