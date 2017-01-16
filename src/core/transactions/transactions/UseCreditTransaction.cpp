#include "UseCreditTransaction.h"

UseCreditTransaction::UseCreditTransaction(
        UseCreditCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::UseCreditTransactionType),
        mCommand(command) {}

UseCreditCommand::Shared UseCreditTransaction::command() const {
    return mCommand;
}

void UseCreditTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> UseCreditTransaction::serializeContext() {

}

TransactionResult::Shared UseCreditTransaction::run() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(new TransactionState(10)));
    return TransactionResult::Shared(transactionResult);
}
