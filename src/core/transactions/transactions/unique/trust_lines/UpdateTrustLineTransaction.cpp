#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    const NodeUUID &nodeUUID,
    UpdateTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::UpdateTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

    deserializeFromBytes(
        buffer);
}

UpdateTrustLineMessage::Shared UpdateTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst UpdateTrustLineTransaction::run() {

    // Check if CoordinatorUUId is Valid
    if (!isContractorUUIDValid(mMessage->senderUUID))
        // todo add production log
        return resultDone();

    if (!isIncomingTrustLineDirectionExisting())
        // todo add production log
        return resultDone();

    if (!isAmountValid(mMessage->newAmount()))
        // todo add production log
        return resultDone();

    // check if  Trustline is available for delete
    updateIncomingTrustAmount();
    logUpdatingTrustLineOperation();

    updateIncomingTrustAmount();
    return resultDone();
}

bool UpdateTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->senderUUID, TrustLineDirection::Incoming) ||
           mTrustLinesManager->checkDirection(mMessage->senderUUID, TrustLineDirection::Both);
}

void UpdateTrustLineTransaction::updateIncomingTrustAmount() {

    mTrustLinesManager->setIncomingTrustAmount(
        mMessage->senderUUID,
        mMessage->newAmount());
}

void UpdateTrustLineTransaction::logUpdatingTrustLineOperation() {

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Updating,
        mMessage->senderUUID,
        mMessage->newAmount());

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
    ioTransaction->trustLineHandler()->saveTrustLine(
        mTrustLinesManager->trustLines().at(
            mMessage->senderUUID));
}

const string UpdateTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[UpdateTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

bool UpdateTrustLineTransaction::isAmountValid(const TrustLineAmount &amount){
    return amount > TrustLine::kZeroAmount();
}