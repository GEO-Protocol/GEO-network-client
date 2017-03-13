#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        nodeUUID
    ),
    mCommand(command),
    mTrustLinesManager(manager) {}

SetTrustLineTransaction::SetTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType
    ),
    mTrustLinesManager(manager) {

    deserializeFromBytes(buffer);
}

SetTrustLineCommand::Shared SetTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> SetTrustLineTransaction::serializeToBytes() const{

    auto parentBytesAndCount = TrustLineTransaction::serializeToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +  commandBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        commandBytesAndCount.first.get(),
        commandBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void SetTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    TrustLineTransaction::deserializeFromBytes(buffer);
    BytesShared commandBufferShared = tryCalloc(SetTrustLineCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
        commandBufferShared.get(),
        buffer.get() + TrustLineTransaction::kOffsetToDataBytes(),
        SetTrustLineCommand::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mCommand = SetTrustLineCommand::Shared(
        new SetTrustLineCommand(
            commandBufferShared
        )
    );
}

TransactionResult::SharedConst SetTrustLineTransaction::run() {

    try {
        switch (mStep) {

            case 1: {
                if (!isTransactionToContractorUnique()) {
                    return conflictErrorResult();
                }
                increaseStepsCounter();
            }

            case 2: {
                if (!isOutgoingTrustLineDirectionExisting()) {
                    return trustLineAbsentResult();
                }
                increaseStepsCounter();
            }

        case 3: {
            if (!mContext.empty()) {
                return checkTransactionContext();

                } else {
                    if (mRequestCounter < kMaxRequestsCount) {
                        increaseRequestsCounter();
                        sendMessageToRemoteNode();
                    } else {
                        return noResponseResult();
                    }
                }
                return waitingForResponseState();
            }

            default: {
                throw ConflictError("SetTrustLineTransaction::run: "
                                        "Illegal step execution.");
            }

        }

    } catch (exception &e) {
        throw RuntimeError("SetTrustLineTransaction::run: "
                               "TransactionUUID -> " + mTransactionUUID.stringUUID() + ". " +
                               "Crashed at step -> " + to_string(mStep) + ". "
                               "Message -> " + e.what());
    }
}

bool SetTrustLineTransaction::isTransactionToContractorUnique() {

    return true;
}

bool SetTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
           mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

TransactionResult::SharedConst SetTrustLineTransaction::checkTransactionContext() {

    if (mExpectationResponsesCount == mContext.size()) {
        auto responseMessage = mContext[kResponsePosition];
        if (responseMessage->typeID() == Message::MessageTypeID::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(responseMessage);
            switch (response->code()) {

                case UpdateTrustLineMessage::kResultCodeAccepted: {
                    setOutgoingTrustAmount();
                    return resultOk();
                }

                case UpdateTrustLineMessage::kResultCodeRejected: {
                    return trustLineAbsentResult();
                }

                case UpdateTrustLineMessage::kResultCodeConflict: {
                    return conflictErrorResult();
                }

                case UpdateTrustLineMessage::kResultCodeTransactionConflict: {
                    return transactionConflictResult();
                }

                default: {
                    return unexpectedErrorResult();
                }

            }
        }

        return unexpectedErrorResult();

    } else {
        throw ConflictError("SetTrustLineTransaction::checkTransactionContext: "
                                "Unexpected context size."
        );
    }
}

void SetTrustLineTransaction::sendMessageToRemoteNode() {

    Message *message = new SetTrustLineMessage(
        mNodeUUID,
        mTransactionUUID,
        mCommand->newAmount()
    );

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );
}

TransactionResult::SharedConst SetTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(kConnectionTimeout * 1000)
        ),
        Message::MessageTypeID::ResponseMessageType,
        false
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::SharedConst(transactionState));
    return TransactionResult::SharedConst(transactionResult);
}

void SetTrustLineTransaction::setOutgoingTrustAmount() {

    mTrustLinesManager->setOutgoingTrustAmount(
        mCommand->contractorUUID(),
        mCommand->newAmount()
    );
}

TransactionResult::SharedConst SetTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst SetTrustLineTransaction::trustLineAbsentResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst SetTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst SetTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst SetTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::SharedConst SetTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}