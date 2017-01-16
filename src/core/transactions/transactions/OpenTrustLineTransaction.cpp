#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    OpenTrustLineCommand::Shared command,
    Communicator *communicator,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::OpenTrustLineTransactionType, scheduler),
    mCommand(command),
    mCommunicator(communicator),
    mTrustLinesInterface(interface) {}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {
    return mCommand;
}

void OpenTrustLineTransaction::setContext(
    Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> OpenTrustLineTransaction::serializeContext() {}

TransactionResult::Shared OpenTrustLineTransaction::run() {

    //// STEP 1
    auto *transactions = pendingTransactions();
    for (auto const &it : *transactions) {

        switch (it.first->type()) {

            case BaseTransaction::TransactionType::OpenTrustLineTransactionType: {
                OpenTrustLineTransaction::Shared openTrustLineTransaction = static_pointer_cast<OpenTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == openTrustLineTransaction->command()->contractorUUID()) {
                    return conflictErrorResult();
                }
                break;
            }

            case BaseTransaction::TransactionType::SetTrustLineTransactionType: {
                break;
            }

            case BaseTransaction::TransactionType::CloseTrustLineTransactionType: {
                CloseTrustLineTransaction::Shared closeTrustLineTransaction = static_pointer_cast<CloseTrustLineTransaction>(it.first);
                if (mCommand->contractorUUID() == closeTrustLineTransaction->command()->contractorUUID()) {
                    return conflictErrorResult();
                }
                break;
            }

            default: {
                break;
            }

        }
    }

    //// STEP 1.1
    if (mTrustLinesInterface->isExist(mCommand->contractorUUID())) {
        return trustLinePresentResult();
    }

    //// STEP 2
    Message *message = new OpenTrustLineMessage(
        mCommunicator->nodeUUID(),
        uuid(),
        mCommand->amount()
    );

    mCommunicator->sendMessage(
        Message::Shared(message),
        mCommand->contractorUUID()
    );

    return resultOk();

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


