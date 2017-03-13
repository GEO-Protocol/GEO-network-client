#include "BaseTransaction.h"


BaseTransaction::BaseTransaction(
    const BaseTransaction::TransactionType type,
    Logger *log) :

    mType(type),
    mLog(log)
{}

    mFileLogger = unique_ptr<FileLogger>(new FileLogger);
}

BaseTransaction::BaseTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    Logger *log) :

    mType(type),
    mLog(log),
    mTransactionUUID(transactionUUID)
{}

    mFileLogger = unique_ptr<FileLogger>(new FileLogger);
}

BaseTransaction::BaseTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    Logger *log) :

    mType(type),
    mLog(log),
    mNodeUUID(nodeUUID)
{}

BaseTransaction::BaseTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID,
    Logger *log) :

    mType(type),
    mLog(log),
    mTransactionUUID(transactionUUID),
    mNodeUUID(nodeUUID)
{}

void BaseTransaction::addMessage(
    Message::Shared message,
    const NodeUUID &nodeUUID) {

    outgoingMessageIsReadySignal(
        message,
        nodeUUID);
}

void BaseTransaction::launchSubsidiaryTransaction(
    BaseTransaction::Shared transaction) {

    runSubsidiaryTransactionSignal(
        transaction
    );
}

TransactionResult::Shared BaseTransaction::resultExit()
{
    return make_shared<TransactionResult>(
        TransactionState::exit());
}

TransactionResult::Shared BaseTransaction::resultFlushAndContinue()
{
    return make_shared<TransactionResult>(
        TransactionState::flushAndContinue());
}

TransactionResult::Shared BaseTransaction::resultWaitForMessageTypes(
    vector<Message::MessageTypeID> &&requiredMessagesTypes,
    uint16_t noLongerThanMilliseconds)
{
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes(
            move(requiredMessagesTypes),
            noLongerThanMilliseconds));
}

const BaseTransaction::TransactionType BaseTransaction::transactionType() const {

    return mType;
}

const TransactionUUID &BaseTransaction::UUID() const {

    return mTransactionUUID;
}

const NodeUUID &BaseTransaction::nodeUUID() const {

    return mNodeUUID;
}

void BaseTransaction::increaseStepsCounter() {

    mStep += 1;
}

void BaseTransaction::resetStepsCounter() {

    mStep = 1;
}

void BaseTransaction::setExpectationResponsesCounter(
    uint16_t count) {

    mExpectationResponsesCount = count;
}

void BaseTransaction::resetExpectationResponsesCounter() {

    mExpectationResponsesCount = 0;
}

void BaseTransaction::pushContext(
    Message::Shared message) {

    mContext.push_back(message);
}

void BaseTransaction::clearContext() {

    mContext.clear();
}

pair<BytesShared, size_t> BaseTransaction::serializeToBytes() const {

    size_t bytesCount = sizeof(SerializedTransactionType) +
        NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        sizeof(uint16_t);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //-----------------------------------------------------
    uint16_t transactionType = mType;
    memcpy(
        dataBytesShared.get(),
        &transactionType,
        sizeof(SerializedTransactionType)
    );
    dataBytesOffset += sizeof(SerializedTransactionType);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    dataBytesOffset += TransactionUUID::kBytesSize;
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mNodeUUID.data,
        NodeUUID::kBytesSize
    );
    dataBytesOffset += NodeUUID::kBytesSize;
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mStep,
        sizeof(uint16_t)
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void BaseTransaction::deserializeFromBytes(
    BytesShared buffer) {

    size_t bytesBufferOffset = 0;

    SerializedTransactionType *transactionType = new (buffer.get()) SerializedTransactionType;
    mType = (TransactionType) *transactionType;
    bytesBufferOffset += sizeof(SerializedTransactionType);
    //-----------------------------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer.get() + bytesBufferOffset,
        TransactionUUID::kBytesSize
    );
    bytesBufferOffset += TransactionUUID::kBytesSize;
    //-----------------------------------------------------
    memcpy(
        mNodeUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    //-----------------------------------------------------
    uint16_t *step = new (buffer.get() + bytesBufferOffset) uint16_t;
    mStep = *step;
}

const size_t BaseTransaction::kOffsetToInheritedBytes() {

    static const size_t offset = sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + sizeof(uint16_t);
    return offset;
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromCommand(
    CommandResult::SharedConst result) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(result);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromMessage(
    MessageResult::SharedConst messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromState(
    TransactionState::SharedConst state) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(state);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::finishTransaction() {

    return make_shared<const TransactionResult>(
        TransactionState::exit()
    );
}

const string BaseTransaction::logHeader() const
{
    // todo: must be marked as "=0" in header;
}

LoggerStream BaseTransaction::info() const
{
    // TODO: remove me. Logger must be initialised in constructor by default
    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->info(logHeader());
}

LoggerStream BaseTransaction::error() const
{
    // TODO: remove me. Logger must be initialised in constructor by default
    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->error(logHeader());
}

LoggerStream BaseTransaction::debug() const
{
    // TODO: remove me. Logger must be initialised in constructor by default
    if (nullptr == mLog)
        throw Exception("logger is not initialised");

    return mLog->debug(logHeader());
}
