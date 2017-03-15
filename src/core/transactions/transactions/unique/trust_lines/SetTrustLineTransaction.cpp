#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        nodeUUID),
    mCommand(command),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {}

SetTrustLineTransaction::SetTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    OperationsHistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType),
    mTrustLinesManager(manager),
    mOperationsHistoryStorage(historyStorage) {

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

    try {
        switch (mStep) {

            case Stages::CheckUnicity: {
                if (!isTransactionToContractorUnique()) {
                    return conflictErrorResult();
                }

                mStep = Stages::CheckOutgoingDirection;
            }

            case Stages::CheckOutgoingDirection: {
                if (!isOutgoingTrustLineDirectionExisting()) {
                    return trustLineAbsentResult();
                }

                mStep = Stages::CheckContext;
            }

        case Stages::CheckContext: {
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
                               "Message -> " + string(e.what()));
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
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageTypeID::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(
                responseMessage);

            switch (response->code()) {

                case UpdateTrustLineMessage::kResultCodeAccepted: {
                    setOutgoingTrustAmount();
                    logSetTrustLineOperation();

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
                                "Unexpected context size.");
    }
}

void SetTrustLineTransaction::sendMessageToRemoteNode() {

    sendMessage<SetTrustLineMessage>(
        mCommand->contractorUUID(),
        mNodeUUID,
        mTransactionUUID,
        mCommand->newAmount());
}

TransactionResult::SharedConst SetTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        microsecondsSinceGEOEpoch(
            utc_now() + pt::microseconds(kConnectionTimeout * 1000)),
        Message::MessageTypeID::ResponseMessageType,
        false);


    return transactionResultFromState(
        TransactionState::SharedConst(
            transactionState));
}

void SetTrustLineTransaction::setOutgoingTrustAmount() {

    mTrustLinesManager->setOutgoingTrustAmount(
        mCommand->contractorUUID(),
        mCommand->newAmount());
}

void SetTrustLineTransaction::logSetTrustLineOperation() {

    Record::Shared record = make_shared<TrustLineRecord>(
        uuid(mTransactionUUID),
        TrustLineRecord::TrustLineOperationType::Setting,
        mCommand->contractorUUID(),
        mCommand->newAmount());

    mOperationsHistoryStorage->addRecord(
        record);
}

TransactionResult::SharedConst SetTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(
        mCommand->resultOk());
}

TransactionResult::SharedConst SetTrustLineTransaction::trustLineAbsentResult() {

    return transactionResultFromCommand(
        mCommand->trustLineAbsentResult());
}

TransactionResult::SharedConst SetTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(
        mCommand->resultConflict());
}

TransactionResult::SharedConst SetTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(
        mCommand->resultNoResponse());
}

TransactionResult::SharedConst SetTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(
        mCommand->resultTransactionConflict());
}

TransactionResult::SharedConst SetTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(
        mCommand->unexpectedErrorResult());
}