#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    OpenTrustLineCommand::Shared command,
    Communicator *communicator,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::OpenTrustLineTransactionType, scheduler),
    mCommand(command),
    mCommunicator(communicator),
    mTrustLinesInterface(interface) {

    mStep = 1;
    mRequestCounter = 0;
}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {
    return mCommand;
}

void OpenTrustLineTransaction::setContext(
    Message::Shared message) {

    mContext = message;
}

pair<byte *, size_t> OpenTrustLineTransaction::serializeContext() {}

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
                if (mContext->typeID() == Message::MessageTypeID::ResponseMessageType) {
                    Response::Shared response = static_pointer_cast<Response>(mContext);
                    cout << "Transaction result code " << to_string(response->mCode) << endl;
                }

            } else {
                if (mRequestCounter < kMaxRequestsCount) {
                    sendMessageToRemoteNode();
                    mRequestCounter += 1;

                } else {
                    return noResponseResult();
                }
            }
            return waitForResponse();
        }

        default: {
            break;
        }

    }

    return resultOk();

}

bool OpenTrustLineTransaction::checkSameTypeTransactions() {

    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->type()) {

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

bool OpenTrustLineTransaction::checkTrustLineDirection() {

    return mTrustLinesInterface->isDirectionOutgoing(mCommand->contractorUUID());
}

void OpenTrustLineTransaction::sendMessageToRemoteNode() {

    Message *message = new OpenTrustLineMessage(
        mCommunicator->nodeUUID(),
        uuid(),
        mCommand->amount()
    );

    mCommunicator->sendMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );
}

TransactionResult::Shared OpenTrustLineTransaction::waitForResponse() {

    TransactionState *transactionState = new TransactionState(
        kConnectionTimeout,
        Message::MessageTypeID::ResponseMessageType
    );


    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(transactionState));
    return TransactionResult::Shared(transactionResult);
}

void OpenTrustLineTransaction::increaseStepsCounter() {

    mStep += 1;
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



