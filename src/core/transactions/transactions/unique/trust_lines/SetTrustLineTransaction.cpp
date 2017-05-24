#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {}

SetTrustLineTransaction::SetTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger *logger) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        logger),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler) {

    deserializeFromBytes(
        buffer);
}

SetTrustLineCommand::Shared SetTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> SetTrustLineTransaction::serializeToBytes() const{

    auto parentBytesAndCount = TrustLineTransaction::serializeToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + commandBytesAndCount.second;

    BytesShared dataBytesShared = tryMalloc(
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

void SetTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(
        buffer);

    BytesShared commandBufferShared = tryMalloc(
        SetTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        commandBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        SetTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    mCommand = make_shared<SetTrustLineCommand>(
        commandBufferShared);
}

TransactionResult::SharedConst SetTrustLineTransaction::run() {

    // Check if CoordinatorUUId is Valid
    if (!isContractorUUIDValid(mCommand->contractorUUID()))
        return responseProtocolError();

    // Check if TrustLine exist
    if (!isOutgoingTrustLineDirectionExisting()) {
        return responseTrustlineIsAbsent();
    }

    // Check if amount correct
    if (!isAmountValid(mCommand->newAmount()))
        return responseProtocolError();
    // Notify Contractor that trustLine will be be closed
    sendMessageToRemoteNode();

    // check if  TrustLine is available for delete
    setOutgoingTrustAmount();
    logSetTrustLineOperation();
    return responseOk();
}

bool SetTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool SetTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
           mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

void SetTrustLineTransaction::sendMessageToRemoteNode() {

    sendMessage<SetTrustLineMessage>(
        mCommand->contractorUUID(),
        mNodeUUID,
        mTransactionUUID,
        mCommand->newAmount());
}

void SetTrustLineTransaction::setOutgoingTrustAmount() {

    mTrustLinesManager->setOutgoingTrustAmount(
        mCommand->contractorUUID(),
        mCommand->newAmount());
}

void SetTrustLineTransaction::logSetTrustLineOperation() {

    TrustLineRecord::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Setting,
        mCommand->contractorUUID(),
        mCommand->newAmount());

    auto ioTransaction = mStorageHandler->beginTransaction();
    ioTransaction->historyStorage()->saveTrustLineRecord(record);
    ioTransaction->trustLineHandler()->saveTrustLine(
        mTrustLinesManager->trustLines().at(
            mCommand->contractorUUID()));
}

TransactionResult::SharedConst SetTrustLineTransaction::responseOk() {

    return transactionResultFromCommand(
            mCommand->responseCreated());
}

TransactionResult::SharedConst SetTrustLineTransaction::responseTrustlineIsAbsent() {

    return transactionResultFromCommand(
            mCommand->responseTrustlineIsAbsent());
}

TransactionResult::SharedConst SetTrustLineTransaction::responseProtocolError() {
    return transactionResultFromCommand(
            mCommand->responseProtocolError());
}

const string SetTrustLineTransaction::logHeader() const
{
    stringstream s;
    s << "[SetTrustLineTA: " << currentTransactionUUID() << "]";
    return s.str();
}

bool SetTrustLineTransaction::isAmountValid(const TrustLineAmount &amount){
    return amount > TrustLine::kZeroAmount();
}