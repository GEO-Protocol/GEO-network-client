#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
    NodeUUID &nodeUUID,
    CloseTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
        BaseTransaction::TransactionType::CloseTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mCommand(command),
    mTrustLinesManager(manager){

}

CloseTrustLineTransaction::CloseTrustLineTransaction(
    BytesShared buffer,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(scheduler),
    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> CloseTrustLineTransaction::serializeToBytes() {

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

void CloseTrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    deserializeParentFromBytes(buffer);
    byte *commandBuffer = (byte *) calloc(
        CloseTrustLineCommand::kRequestedBufferSize(),
        sizeof(byte)
    );
    memcpy(
        commandBuffer,
        buffer.get() + kOffsetToDataBytes(),
        CloseTrustLineCommand::kRequestedBufferSize()
    );
    BytesShared commandBufferShared(commandBuffer, free);
    CloseTrustLineCommand *command = new CloseTrustLineCommand(commandBufferShared);
    mCommand = CloseTrustLineCommand::Shared(command);

}

TransactionResult::Shared CloseTrustLineTransaction::run() {

    switch(mStep) {

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
            if (checkDebt()) {
                suspendTrustLineToContractor();

            } else {
                closeTrustLine();
            }
            increaseStepsCounter();
        }

        case 4: {
            if (mContext.get() != nullptr) {
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
            throw ConflictError("CloseTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }

}

bool CloseTrustLineTransaction::checkSameTypeTransactions() {

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

bool CloseTrustLineTransaction::checkTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing
    );
}

bool CloseTrustLineTransaction::checkDebt() {

    return mTrustLinesManager->balanceRange(mCommand->contractorUUID()) == BalanceRange::Positive;
}

void CloseTrustLineTransaction::suspendTrustLineToContractor() {

    mTrustLinesManager->suspendDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing
    );
}

void CloseTrustLineTransaction::closeTrustLine() {

    mTrustLinesManager->close(mCommand->contractorUUID());
}

TransactionResult::Shared CloseTrustLineTransaction::checkTransactionContext() {

    if (mContext->typeID() == Message::MessageTypeID::ResponseMessageType) {
        Response::Shared response = static_pointer_cast<Response>(mContext);
        switch (response->code()) {

            case RejectTrustLineMessage::kResultCodeRejected: {
                return resultOk();
            }

            case RejectTrustLineMessage::kResultCodeRejectDelayed: {
                return resultOk();
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
}

void CloseTrustLineTransaction::sendMessageToRemoteNode() {

    Message *message = new CloseTrustLineMessage(
        mNodeUUID,
        mTransactionUUID,
        mNodeUUID
    );

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );
}

TransactionResult::Shared CloseTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        kConnectionTimeout,
        Message::MessageTypeID::ResponseMessageType,
        false
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared CloseTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultOk())));
}

TransactionResult::Shared CloseTrustLineTransaction::trustLineAbsentResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->trustLineIsAbsentResult())));
}

TransactionResult::Shared CloseTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultConflict())));
}

TransactionResult::Shared CloseTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultNoResponse())));
}

TransactionResult::Shared CloseTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultTransactionConflict())));
}

TransactionResult::Shared CloseTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->unexpectedErrorResult())));
}