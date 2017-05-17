#include "RejectTrustLineTransaction.h"

RejectTrustLineTransaction::RejectTrustLineTransaction(
    const NodeUUID &nodeUUID,
    RejectTrustLineMessage::Shared message,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}


RejectTrustLineTransaction::RejectTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::RejectTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

    deserializeFromBytes(
        buffer);
}

RejectTrustLineMessage::Shared RejectTrustLineTransaction::message() const {

    return mMessage;
}

TransactionResult::SharedConst RejectTrustLineTransaction::run() {

    // Check if CoordinatorUUId is Valid
    if (!isContractorUUIDValid(mMessage->contractorUUID()))
        return transactionResultFromMessage(mMessage->resultRejected());

    if (!isIncomingTrustLineDirectionExisting())
        return transactionResultFromMessage(
            mMessage->resultRejected());
    // check if  Trustline is available for delete
    if (trustLineIsAvailableForDelete()) {
        // close trustline
        rejectTrustLine();
        // update routing tables;
        if (!mTrustLinesManager->isNeighbor(mMessage->contractorUUID())) {
            const auto kTransaction = make_shared<TrustLineStatesHandlerTransaction>(
                currentNodeUUID(),
                currentNodeUUID(),
                currentNodeUUID(),
                mMessage->contractorUUID(),
                TrustLineStatesHandlerTransaction::TrustLineState::Removed,
                0,
                mTrustLinesManager,
                mStorageHandler,
                mLog);
            launchSubsidiaryTransaction(kTransaction);
        }
        return resultDone();
    } else {
        mTrustLinesManager->setOutgoingTrustAmount(mMessage->contractorUUID(), 0);
        return resultDone();
    }
}

bool RejectTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool RejectTrustLineTransaction::isIncomingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Incoming) ||
        mTrustLinesManager->checkDirection(mMessage->contractorUUID(), TrustLineDirection::Both);
}

void RejectTrustLineTransaction::rejectTrustLine() {

    mTrustLinesManager->reject(
        mMessage->contractorUUID());
}

void RejectTrustLineTransaction::logRejectingTrustLineOperation() {

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Rejecting,
        mMessage->senderUUID);

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}

bool RejectTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mMessage->contractorUUID()) == BalanceRange::Negative;
}

void RejectTrustLineTransaction::sendResponseCodeToContractor(
    const uint16_t code) {

    sendMessage<Response>(
        mMessage->senderUUID,
        mNodeUUID,
        mMessage->transactionUUID(),
        code);
}

const string RejectTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[RejectTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

bool RejectTrustLineTransaction::trustLineIsAvailableForDelete() {
    const auto zeroBalance = TrustLineBalance(0);
    const auto zeroAmount = TrustLineAmount(0);

    if (mTrustLinesManager->balance(mMessage->contractorUUID()) == zeroBalance
        and mTrustLinesManager->outgoingTrustAmount(mMessage->contractorUUID()) == zeroAmount
        and mTrustLinesManager->incomingTrustAmount(mMessage->contractorUUID()) == zeroAmount
        and not mTrustLinesManager->reservationIsPresent(mMessage->contractorUUID())){
        return true;
    }
    return false;
}
