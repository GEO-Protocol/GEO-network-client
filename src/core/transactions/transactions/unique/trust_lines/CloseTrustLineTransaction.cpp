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
    switch (mStep) {

        case Stages::CheckUnicity: {
            if (!isTransactionToContractorUnique()) {
                return resultConflictWithOtherOperation();
            }

            mStep = Stages::CheckOutgoingDirection;
        }

        case Stages::CheckOutgoingDirection: {
            if (!isOutgoingTrustLineDirectionExisting()) {
                return resultTrustLineAbsent();
            }

            mStep = Stages::CheckDebt;
        }

        case Stages::CheckDebt: {
            if (checkDebt()) {
                suspendTrustLineDirectionToContractor();

            } else {
                closeTrustLine();
                logClosingTrustLineOperation();
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
                    return resultRemoteNodeIsInaccessible();
                }

            }

            return waitingForResponseState();
        }

        default: {
            throw ConflictError("CloseTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

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

void CloseTrustLineTransaction::suspendTrustLineDirectionToContractor() {

    mTrustLinesManager->suspendDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing);
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

TransactionResult::SharedConst CloseTrustLineTransaction::checkTransactionContext() {

    if (mkExpectationResponsesCount == mContext.size()) {
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageType::ResponseMessageType) {
            Response::Shared response = static_pointer_cast<Response>(
                responseMessage);

            switch (response->code()) {

                case RejectTrustLineMessage::kResultCodeRejected: {
                    return resultOk();
                }

                case RejectTrustLineMessage::kResultCodeRejectDelayed: {
                    return resultOk();
                }

                case RejectTrustLineMessage::kResultCodeTrustLineAbsent: {
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
        throw ConflictError("CloseTrustLineTransaction::checkTransactionContext: "
                                "Unexpected context size.");
    }
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

TransactionResult::SharedConst CloseTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(
        mCommand->responseOK());
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