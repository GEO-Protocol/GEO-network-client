/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "RejectOutgoingTrustLineTransaction.h"

RejectOutgoingTrustLineTransaction::RejectOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    ConfirmationMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::RejectOutgoingTrustLineTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst RejectOutgoingTrustLineTransaction::run()
{
    const auto kContractor = mMessage->senderUUID;

    if (kContractor == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLinesManager->closeOutgoing(
            ioTransaction,
            kContractor);
        auto record = make_shared<TrustLineRecord>(
            currentTransactionUUID(),
            TrustLineRecord::RejectingOutgoing,
            kContractor);

        ioTransaction->historyStorage()->saveTrustLineRecord(record);

        processConfirmationMessage(
            kContractor,
            mMessage);
        info() << "Outgoing TL to the node " << kContractor << " successfully rejected";
        return resultDone();
    } catch (NotFoundError &e) {
        warning() << "Can't reject outgoing TL because absence. Details are: " << e.what();
        processConfirmationMessage(
            kContractor,
            mMessage);
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to close outgoing trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish propely and no result may be returned.
        throw e;
    }
}

const string RejectOutgoingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[RejectOutgoingTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}
