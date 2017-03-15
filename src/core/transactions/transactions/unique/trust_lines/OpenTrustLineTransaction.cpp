#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    OpenTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::OpenTrustLineTransactionType,
        nodeUUID),
    mCommand(command),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {}

OpenTrustLineTransaction::OpenTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::OpenTrustLineTransactionType),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {

    deserializeFromBytes(
        buffer);
}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> OpenTrustLineTransaction::serializeToBytes() const {

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

void OpenTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(
        buffer);

    BytesShared commandBufferShared = tryMalloc(
        OpenTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        commandBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        OpenTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    mCommand = make_shared<OpenTrustLineCommand>(
        commandBufferShared);
}

TransactionResult::SharedConst OpenTrustLineTransaction::run() {

    try {
        switch (mStep) {

            case Stages::CheckUnicity: {
                if (!isTransactionToContractorUnique()) {
                    return conflictErrorResult();
                }

                mStep = Stages::CheckOutgoingDirection;
            }

            case Stages::CheckOutgoingDirection: {
                if (isOutgoingTrustLineDirectionExisting()) {
                    return trustLinePresentResult();
                }

                mStep = Stages::CheckContext;
            }

        case Stages::CheckContext: {
            if (!mContext.empty()) {
                return checkTransactionContext();

                } else {

                    if (mRequestCounter < kMaxRequestsCount) {
                        sendMessageToRemoteNode();
                        increaseRequestsCounter();

                    } else {
                        return noResponseResult();
                    }

                }
                return waitingForResponseState();
            }

            default: {
                throw ConflictError("OpenTrustLineTransaction::run: "
                                        "Illegal step execution.");
            }

        }

    } catch (exception &e){
        throw RuntimeError("OpenTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + e.what());
    }
}

bool OpenTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool OpenTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
        mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

TransactionResult::SharedConst OpenTrustLineTransaction::checkTransactionContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageTypeID::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(
                responseMessage);

            switch (response->code()) {

                case AcceptTrustLineMessage::kResultCodeAccepted: {
                    openTrustLine();
                    logOpeningTrustLineOperation();

                    return resultOk();
                }

                case AcceptTrustLineMessage::kResultCodeConflict: {
                    return conflictErrorResult();
                }

                case AcceptTrustLineMessage::kResultCodeTransactionConflict: {
                    return transactionConflictResult();
                }

                default:{
                    return unexpectedErrorResult();
                }

            }

        }

        return unexpectedErrorResult();

    } else {
        throw ConflictError("OpenTrustLineTransaction::checkTransactionContext: "
                                "Unexpected context size.");
    }
}

void OpenTrustLineTransaction::sendMessageToRemoteNode() {

    sendMessage<OpenTrustLineMessage>(
        mCommand->contractorUUID(),
        mNodeUUID,
        mTransactionUUID,
        mCommand->amount());
}

TransactionResult::SharedConst OpenTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(kConnectionTimeout * 1000)),
        Message::MessageTypeID::ResponseMessageType,
        false);

    return transactionResultFromState(
        TransactionState::SharedConst(
            transactionState));
}

void OpenTrustLineTransaction::openTrustLine() {

    mTrustLinesManager->open(
        mCommand->contractorUUID(),
        mCommand->amount());

}

void OpenTrustLineTransaction::logOpeningTrustLineOperation() {

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Opening,
        mCommand->contractorUUID(),
        mCommand->amount());

    mOperationsHistoryStorage->addRecord(
        record);
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(
        mCommand->resultOk());
}

TransactionResult::SharedConst OpenTrustLineTransaction::trustLinePresentResult() {

    return transactionResultFromCommand(
        mCommand->trustLineAlreadyPresentResult());
}

TransactionResult::SharedConst OpenTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(
        mCommand->resultConflict());
}

TransactionResult::SharedConst OpenTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(
        mCommand->resultNoResponse());
}

TransactionResult::SharedConst OpenTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(
        mCommand->resultTransactionConflict());
}

TransactionResult::SharedConst OpenTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(
        mCommand->unexpectedErrorResult());
}