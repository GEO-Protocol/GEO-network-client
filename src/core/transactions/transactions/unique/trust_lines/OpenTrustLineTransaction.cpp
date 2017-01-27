#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    NodeUUID &nodeUUID,
    OpenTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesManager *manager) :

    UniqueTransaction(
        BaseTransaction::TransactionType::OpenTrustLineTransactionType,
        nodeUUID,
        scheduler
    ),
    mCommand(command),
    mTrustLinesManager(manager) {}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {

    return mCommand;
}

TransactionResult::Shared OpenTrustLineTransaction::run() {

    switch (mStep) {

        case 1: {
            if (checkSameTypeTransactions()) {
                return conflictErrorResult();
            }
            increaseStepsCounter();
        }

        case 2: {
            if (checkTrustLineDirectionExisting()) {
                return trustLinePresentResult();
            }
            increaseStepsCounter();
        }

        case 3: {
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
            throw ConflictError("OpenTrustLineTransaction::run: "
                                    "Illegal step execution.");
        }

    }
}

bool OpenTrustLineTransaction::checkSameTypeTransactions() {

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

bool OpenTrustLineTransaction::checkTrustLineDirectionExisting() {

    return mTrustLinesManager->checkDirection(
        mCommand->contractorUUID(),
        TrustLineDirection::Outgoing
    );
}

TransactionResult::Shared OpenTrustLineTransaction::checkTransactionContext() {

    if (mContext->typeID() == Message::MessageTypeID::ResponseMessageType) {
        Response::Shared response = static_pointer_cast<Response>(mContext);
        switch (response->code()) {

            case AcceptTrustLineMessage::kResultCodeAccepted: {
                openTrustLine();
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

}

void OpenTrustLineTransaction::sendMessageToRemoteNode() {

    Message *message = new OpenTrustLineMessage(
        mNodeUUID,
        mTransactionUUID,
        mCommand->amount()
    );

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );
}

TransactionResult::Shared OpenTrustLineTransaction::waitingForResponseState() {

    TransactionState *transactionState = new TransactionState(
        kConnectionTimeout,
        Message::MessageTypeID::ResponseMessageType,
        false
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

void OpenTrustLineTransaction::openTrustLine() {

    try {
        mTrustLinesManager->open(
            mCommand->contractorUUID(),
            mCommand->amount()
        );

    } catch (std::exception &e) {
        throw Exception(e.what());
    }
}

TransactionResult::Shared OpenTrustLineTransaction::resultOk() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultOk())));
}

TransactionResult::Shared OpenTrustLineTransaction::trustLinePresentResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->trustLineAlreadyPresentResult())));
}

TransactionResult::Shared OpenTrustLineTransaction::conflictErrorResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultConflict())));
}

TransactionResult::Shared OpenTrustLineTransaction::noResponseResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultNoResponse())));
}

TransactionResult::Shared OpenTrustLineTransaction::transactionConflictResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultTransactionConflict())));
}

TransactionResult::Shared OpenTrustLineTransaction::unexpectedErrorResult() {

    return transactionResultFromCommand(CommandResult::Shared(const_cast<CommandResult *> (mCommand->unexpectedErrorResult())));
}
