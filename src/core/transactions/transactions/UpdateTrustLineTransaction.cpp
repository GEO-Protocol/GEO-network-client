#include "UpdateTrustLineTransaction.h"

UpdateTrustLineTransaction::UpdateTrustLineTransaction(
    UpdateTrustLineCommand::Shared command,
    TransactionsScheduler *scheduler,
    TrustLinesInterface *interface) :

    UniqueTransaction(BaseTransaction::TransactionType::UpdateTrustLineTransactionType, scheduler),
    mCommand(command),
    mTrustLinesInterface(interface) {}

UpdateTrustLineCommand::Shared UpdateTrustLineTransaction::command() const {
    return mCommand;
}

void UpdateTrustLineTransaction::setContext(
    Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> UpdateTrustLineTransaction::serializeContext() {}

TransactionResult::Shared UpdateTrustLineTransaction::run() {

    return conflictErrorResult();
}

TransactionResult::Shared UpdateTrustLineTransaction::conflictErrorResult() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(const_cast<CommandResult *>(mCommand->resultConflict())));
    return TransactionResult::Shared(transactionResult);
}
