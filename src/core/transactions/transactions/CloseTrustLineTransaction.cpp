#include "CloseTrustLineTransaction.h"

CloseTrustLineTransaction::CloseTrustLineTransaction(
    CloseTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::CloseTrustLineTransactionType, scheduler),
    mCommand(command),
    mTrustLinesInterface(interface) {}

CloseTrustLineCommand::Shared CloseTrustLineTransaction::command() const {
    return mCommand;
}

void CloseTrustLineTransaction::setContext(
    Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> CloseTrustLineTransaction::serializeContext() {}

TransactionResult::Shared CloseTrustLineTransaction::run() {

    return conflictErrorResult();
}

TransactionResult::Shared CloseTrustLineTransaction::conflictErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *> (mCommand->resultConflict())));
    return TransactionResult::Shared(transactionResult);
}

