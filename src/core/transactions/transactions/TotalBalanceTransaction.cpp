#include "TotalBalanceTransaction.h"

TotalBalanceTransaction::TotalBalanceTransaction(
        TotalBalanceCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::TotalBalanceTransactionType),
        mCommand(command) {}

TotalBalanceCommand::Shared TotalBalanceTransaction::command() const {
    return mCommand;
}

void TotalBalanceTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<byte *, size_t> TotalBalanceTransaction::serializeContext() {}

TransactionResult::Shared TotalBalanceTransaction::run() {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(TransactionState::Shared(new TransactionState(10)));
    return TransactionResult::Shared(transactionResult);
}
