#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(
        BaseTransaction::TransactionType::SetTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mCommand(command),
    mTrustLinesManager(manager) {}

SetTrustLineTransaction::SetTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    TrustLineTransaction(scheduler),
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

    switch (mStep) {

        case 1: {
            if (isTransactionToContractorUnique()) {
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
            if (mContext.get() != nullptr) {
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
}

bool SetTrustLineTransaction::isTransactionToContractorUnique() {

    auto transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->transactionType()) {

            case BaseTransaction::TransactionType::OpenTrustLineTransactionType: {
                OpenTrustLineTransaction::Shared openTrustLineTransaction = static_pointer_cast<OpenTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == openTrustLineTransaction->command()->contractorUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                SetTrustLineTransaction::Shared setTrustLineTransaction = static_pointer_cast<SetTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == setTrustLineTransaction->command()->contractorUUID()) {
                    return true;
                }
                break;
            }

            case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                CloseTrustLineTransaction::Shared closeTrustLineTransaction = static_pointer_cast<CloseTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == closeTrustLineTransaction->command()->contractorUUID()) {
                    return true;
                }
                break;
            }

            default: {
                break;
            }

        }

    }

    return false;
}

bool SetTrustLineTransaction::isOutgoingTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Outgoing) ||
           mTrustLinesManager->checkDirection(mCommand->contractorUUID(), TrustLineDirection::Both);
}

TransactionResult::SharedConst SetTrustLineTransaction::checkTransactionContext() {

    if (mContext->typeID() == Message::MessageTypeID::ResponseMessageType) {
        Response::Shared response = static_pointer_cast<Response>(mContext);
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
        kConnectionTimeout,
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