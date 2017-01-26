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

SetTrustLineCommand::Shared SetTrustLineTransaction::command() const {

    return mCommand;
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
        Message::MessageTypeID::ResponseMessageType
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

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultOk())));
}

TransactionResult::Shared SetTrustLineTransaction::trustLineAbsentResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->trustLineAbsentResult())));
}

TransactionResult::Shared SetTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultConflict())));
}

TransactionResult::Shared SetTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultNoResponse())));
}

TransactionResult::Shared SetTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultTransactionConflict())));
}

TransactionResult::Shared SetTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->unexpectedErrorResult())));
}
