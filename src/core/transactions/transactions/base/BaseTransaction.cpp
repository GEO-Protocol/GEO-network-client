#include "BaseTransaction.h"


BaseTransaction::BaseTransaction(
    const BaseTransaction::TransactionType type,
    Logger &log) :

    mType(type),
    mLog(log)
{
    mStep = 1;
}

BaseTransaction::BaseTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    Logger &log) :

    mType(type),
    mLog(log),
    mTransactionUUID(transactionUUID)
{
    mStep = 1;
}

BaseTransaction::BaseTransaction(
    BytesShared buffer,
    const NodeUUID &nodeUUID,
    Logger &log) :
    mLog(log),
    mNodeUUID(nodeUUID)
{
    deserializeFromBytes(buffer);
}

BaseTransaction::BaseTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    Logger &log) :

    mType(type),
    mLog(log),
    mNodeUUID(nodeUUID)
{
    mStep = 1;
}

BaseTransaction::BaseTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID,
    Logger &log) :

    mType(type),
    mLog(log),
    mTransactionUUID(transactionUUID),
    mNodeUUID(nodeUUID)
{
    mStep = 1;
}

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

TransactionResult::Shared BaseTransaction::resultDone () const
{
    return make_shared<TransactionResult>(
        TransactionState::exit());
}

TransactionResult::Shared BaseTransaction::resultFlushAndContinue() const
{
    return make_shared<TransactionResult>(
        TransactionState::flushAndContinue());
}

// todo Change resultWaitForMessageTypes type to sharedConst
TransactionResult::Shared BaseTransaction::resultWaitForMessageTypes(
    vector<Message::MessageType> &&requiredMessagesTypes,
    uint32_t noLongerThanMilliseconds) const
{
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypes(
            move(requiredMessagesTypes),
            noLongerThanMilliseconds));
}

TransactionResult::Shared BaseTransaction::resultAwaikAfterMilliseconds(
    uint32_t responseWaitTime) const
{
    return make_shared<TransactionResult>(
        TransactionState::awakeAfterMilliseconds(
            responseWaitTime));
}

TransactionResult::Shared BaseTransaction::resultContinuePreviousState() const
{
    return make_shared<TransactionResult>(
        TransactionState::continueWithPreviousState());
}

const BaseTransaction::TransactionType BaseTransaction::transactionType() const {

    return mType;
}

const TransactionUUID &BaseTransaction::currentTransactionUUID () const {

    return mTransactionUUID;
}

const NodeUUID &BaseTransaction::currentNodeUUID () const {

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

    mkExpectationResponsesCount = count;
}

void BaseTransaction::resetExpectationResponsesCounter() {

    mkExpectationResponsesCount = 0;
}

void BaseTransaction::pushContext(
    Message::Shared message) {

    mContext.push_back(message);
}

void BaseTransaction::pushResource(
    BaseResource::Shared resource) {

    mResources.push_back(resource);
}

void BaseTransaction::clearContext() {

    mContext.clear();
}

pair<BytesShared, size_t> BaseTransaction::serializeToBytes() const {
    size_t bytesCount = sizeof(SerializedTransactionType) +
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
    uint16_t *step = new (buffer.get() + bytesBufferOffset) uint16_t;
    mStep = *step;
}

const size_t BaseTransaction::kOffsetToInheritedBytes() {

    static const size_t offset = sizeof(uint16_t) + TransactionUUID::kBytesSize + sizeof(uint16_t);
    return offset;
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromCommand(
    CommandResult::SharedConst result) const {

    // todo: refactor me
    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(result);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromMessage(
    MessageResult::SharedConst messageResult) const {

    // todo: refactor me
    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromState(
    TransactionState::SharedConst state) const {

    // todo: refactor me
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

    return mLog.info(logHeader());
}

LoggerStream BaseTransaction::warning() const
{
    return mLog.warning(logHeader());
}

LoggerStream BaseTransaction::debug() const
{
    return mLog.debug(logHeader());
}

const int BaseTransaction::currentStep() const {
    return mStep;
}

void BaseTransaction::recreateTransactionUUID() {
    mTransactionUUID = TransactionUUID();
}
