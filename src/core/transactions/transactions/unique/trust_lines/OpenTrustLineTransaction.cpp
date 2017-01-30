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
            if (checkTrustLineDirection()) {
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
                break;
            }

            case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                break;
            }

            default: {
                break;
            }

        }

    }

    return false;
}

bool OpenTrustLineTransaction::checkTrustLineDirection() {

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
                createTrustLine();
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
        Message::MessageTypeID::ResponseMessageType
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

void OpenTrustLineTransaction::createTrustLine() {

    try {
        mTrustLinesManager->open(
            mCommand->contractorUUID(),
            mCommand->amount()
        );
        mlogger->logTransactionStatus(mCommand->contractorUUID(), mCommand->amount());
    } catch (std::exception &e) {
        throw Exception(e.what());
    }

}

TransactionResult::Shared OpenTrustLineTransaction::resultOk() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultOk())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared OpenTrustLineTransaction::trustLinePresentResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->trustLineAlreadyPresentResult())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared OpenTrustLineTransaction::conflictErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultConflict())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared OpenTrustLineTransaction::noResponseResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultNoResponse())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared OpenTrustLineTransaction::transactionConflictResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->resultTransactionConflict())));
    return TransactionResult::Shared(transactionResult);
}

TransactionResult::Shared OpenTrustLineTransaction::unexpectedErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand.get()->unexpectedErrorResult())));
    return TransactionResult::Shared(transactionResult);
}
