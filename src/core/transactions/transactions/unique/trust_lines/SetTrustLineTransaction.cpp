#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    HistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        nodeUUID),
    mCommand(command),
    mTrustLinesManager(manager),
    mHistoryStorage(historyStorage) {}

SetTrustLineTransaction::SetTrustLineTransaction(
    BytesShared buffer,
    TrustLinesManager *manager,
    HistoryStorage *historyStorage) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType),
    mTrustLinesManager(manager),
    mHistoryStorage(historyStorage) {

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
                    return resultConflictWithOtherOperation();
                }

                mStep = Stages::CheckOutgoingDirection;
            }

            case Stages::CheckOutgoingDirection: {
                if (!isOutgoingTrustLineDirectionExisting()) {
                    return resultTrustLineIsAbsent();
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
                        return resultRemoteNodeIsInaccessible();
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

    if (mkExpectationResponsesCount == mContext.size()) {
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageType::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(
                responseMessage);

            switch (response->code()) {

                case UpdateTrustLineMessage::kResultCodeAccepted: {
                    setOutgoingTrustAmount();
                    logSetTrustLineOperation();

                    return resultOk();
                }

                case UpdateTrustLineMessage::kResultCodeRejected: {

                    return resultCurrentIncomingDebtIsGreaterThanNewAmount();
                }

                case UpdateTrustLineMessage::kResultCodeTrustLineAbsent: {
                    //todo add TrustLine synchronization
                    throw RuntimeError("CloseTrustLineTransaction::checkTransactionContext:"
                                               "TrustLines data out of sync");
                }

                default: {
                    break;
                }

            }

        }

        return resultProtocolError();

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
        Message::MessageType::ResponseMessageType,
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

    mHistoryStorage->saveRecord(
        record);
    mHistoryStorage->commit();
}

TransactionResult::SharedConst SetTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(
            mCommand->responseCreated());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultTrustLineIsAbsent() {

    return transactionResultFromCommand(
            mCommand->responseTrustlineIsAbsent());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultConflictWithOtherOperation() {

    return transactionResultFromCommand(
            mCommand->responseConflictWithOtherOperation());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultRemoteNodeIsInaccessible() {

    return transactionResultFromCommand(
            mCommand->responseRemoteNodeIsInaccessible());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultProtocolError() {
    return transactionResultFromCommand(
            mCommand->responseProtocolError());
}

TransactionResult::SharedConst SetTrustLineTransaction::resultCurrentIncomingDebtIsGreaterThanNewAmount() {
    return transactionResultFromCommand(
            mCommand->responseCurrentIncomingDebtIsGreaterThanNewAmount());
}
