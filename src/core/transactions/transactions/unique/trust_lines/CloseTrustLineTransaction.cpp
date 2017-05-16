#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::CloseTrustLineTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}

CloseTrustLineTransaction::CloseTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::CloseTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

    deserializeFromBytes(
        buffer);
}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> CloseTrustLineTransaction::serializeToBytes() const{

    auto parentBytesAndCount = TrustLineTransaction::serializeToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + commandBytesAndCount.second;

    BytesShared dataBytesShared = tryCalloc(
        bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        commandBytesAndCount.first.get(),
        commandBytesAndCount.second);
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void CloseTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(
        buffer);

    BytesShared commandBufferShared = tryMalloc(
        CloseTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        commandBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        CloseTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    mCommand = make_shared<CloseTrustLineCommand>(
        commandBufferShared);

}

TransactionResult::SharedConst CloseTrustLineTransaction::run() {

    // Check if CoordinatorUUId is Valid
    if (!isContractorUUIDValid(mCommand->contractorUUID()))
        return resultProtocolError();

    // Check if TrustLine exist
    if (!isOutgoingTrustLineDirectionExisting()) {
        return resultTrustLineAbsent();
    }
    // Notify Contractor that trustline will be be closed
    sendMessageToRemoteNode();

    // Close Trustline is available for delete
    if (trustLineIsAvailableForDelete()) {
        // close trustline
        closeTrustLine();
        logClosingTrustLineOperation();
        // update routing tables;
        if (!mTrustLinesManager->isNeighbor(mCommand->contractorUUID())) {
            const auto kTransaction = make_shared<TrustLineStatesHandlerTransaction>(
                currentNodeUUID(),
                currentNodeUUID(),
                currentNodeUUID(),
                mCommand->contractorUUID(),
                TrustLineStatesHandlerTransaction::TrustLineState::Removed,
                0,
                mTrustLinesManager,
                mStorageHandler,
                mLog);
            launchSubsidiaryTransaction(kTransaction);
        }
        return resultOk(200);
    } else {
        mTrustLinesManager->setOutgoingTrustAmount(mCommand->contractorUUID(), 0);
        return resultOk(202);
    }
}

bool CloseTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool CloseTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
           mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

bool CloseTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mCommand->contractorUUID()) == BalanceRange::Positive;
}

void CloseTrustLineTransaction::closeTrustLine() {

    mTrustLinesManager->close(
        mCommand->contractorUUID());
}

void CloseTrustLineTransaction::logClosingTrustLineOperation() {

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Closing,
        mCommand->contractorUUID());

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
}

void CloseTrustLineTransaction::sendMessageToRemoteNode() {

    sendMessage<CloseTrustLineMessage>(
        mCommand->contractorUUID(),
        mNodeUUID,
        mTransactionUUID,
        mNodeUUID);
}

TransactionResult::SharedConst CloseTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(kConnectionTimeout * 1000)),
        Message::MessageType::ResponseMessageType,
        false);


    return transactionResultFromState(
        TransactionState::SharedConst(transactionState));
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultOk(uint16_t code) {

    return transactionResultFromCommand(
        mCommand->responseOK(code));
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultTrustLineAbsent() {

    return transactionResultFromCommand(
        mCommand->responseTrustlineIsAbsent());
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultConflictWithOtherOperation() {

    return transactionResultFromCommand(
        mCommand->responseConflictWithOtherOperation());
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultRemoteNodeIsInaccessible() {

    return transactionResultFromCommand(
        mCommand->responseRemoteNodeIsInaccessible());
}

TransactionResult::SharedConst CloseTrustLineTransaction::resultProtocolError() {
    return transactionResultFromCommand(
            mCommand->responseProtocolError());
}

const string CloseTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[CloseTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

bool CloseTrustLineTransaction::trustLineIsAvailableForDelete() {
    const auto zeroBalance = TrustLineBalance(0);
    const auto zeroAmount = TrustLineAmount(0);

    if (mTrustLinesManager->balance(mCommand->contractorUUID()) == zeroBalance
        and mTrustLinesManager->outgoingTrustAmount(mCommand->contractorUUID()) == zeroAmount
        and mTrustLinesManager->incomingTrustAmount(mCommand->contractorUUID()) == zeroAmount
        and not mTrustLinesManager->reservationIsPresent(mCommand->contractorUUID())){
        return true;
    }
    return false;
}
