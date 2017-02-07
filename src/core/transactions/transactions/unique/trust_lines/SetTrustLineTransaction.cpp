#include "SetTrustLineTransaction.h"

SetTrustLineTransaction::SetTrustLineTransaction(
    NodeUUID &nodeUUID,
    SetTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
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

    UniqueTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

SetTrustLineCommand::Shared SetTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> SetTrustLineTransaction::serializeToBytes() {

    auto parentBytesAndCount = serializeParentToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +  commandBytesAndCount.second;
    byte *data = (byte *) calloc (
        bytesCount,
        sizeof(byte)
    );
    //-----------------------------------------------------
    memcpy(
        data,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        data + parentBytesAndCount.second,
        commandBytesAndCount.first.get(),
        commandBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        BytesShared(data, free),
        bytesCount
    );
}

void SetTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    deserializeParentFromBytes(buffer);
    byte *commandBuffer = (byte *) calloc(
        SetTrustLineCommand::kRequestedBufferSize(),
        sizeof(byte)
    );
    memcpy(
        commandBuffer,
        buffer.get() + kOffsetToDataBytes(),
        SetTrustLineCommand::kRequestedBufferSize()
    );
    BytesShared commandBufferShared(commandBuffer, free);
    SetTrustLineCommand *command = new SetTrustLineCommand(commandBufferShared);
    mCommand = SetTrustLineCommand::Shared(command);
}

TransactionResult::Shared SetTrustLineTransaction::run() {

    switch (mStep) {

        case 1: {
            if (checkSameTypeTransactions()) {
                return conflictErrorResult();
            }
            increaseStepsCounter();
        }

        case 2: {
            if (!checkTrustLineDirectionExisting()) {
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

bool SetTrustLineTransaction::checkSameTypeTransactions() {

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

bool SetTrustLineTransaction::checkTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing
    );
}

TransactionResult::Shared SetTrustLineTransaction::checkTransactionContext() {

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

            default:{
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

TransactionResult::Shared SetTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        kConnectionTimeout,
        Message::MessageTypeID::ResponseMessageType,
        false
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

void SetTrustLineTransaction::setOutgoingTrustAmount() {

    mTrustLinesManager->setOutgoingTrustAmount(
        mCommand->contractorUUID(),
        mCommand->newAmount()
    );
}

TransactionResult::Shared SetTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::Shared SetTrustLineTransaction::trustLineAbsentResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::Shared SetTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::Shared SetTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::Shared SetTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}

TransactionResult::Shared SetTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(mCommand->resultOk());
}